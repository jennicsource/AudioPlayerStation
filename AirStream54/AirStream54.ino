#include <Arduino.h>

//#define PACKET_LENGTH_SAMPLES 64
//#define PACKET_LENGTH_BYTES   128
//#define SPI_BUFFER_BYTES      132

#include "BoardPin.h"
#include "Radio_Stream_nRF5x.h"
#include "SPIS_nRF54.h"


struct {            
  uint8_t ccommand;         
  uint8_t cvalue;
  uint8_t packetnumber;
  uint8_t fbyte;   
  uint8_t stream[128];
} SPIPacketIn;

struct {            
  uint8_t ccommand;         
  uint8_t cvalue;
  uint8_t packetnumber;
  uint8_t fbyte;   
  uint8_t stream[128];
} SPIPacketOut; 


uint8_t     StreamPacketBytes = 128;
uint16_t    PacketGoodPinTime = 500;
uint8_t     RadioChannel      = 10;
uint8_t     RadioMode         = 0;
uint16_t    SampleRate        = 32000;


void setup() {
    
    Serial.begin(921600);
    delay(500);
    //while (!Serial) delay(10); // Wait for Serial Monitor
    Serial.println("nRF SPI Slave Initializing...");

    pinMode(LED_BUILTIN, OUTPUT);

    SPI_Slave_Init( StreamPacketBytes + 4 );

    pinMode(RF_SW, OUTPUT);
    pinMode(RF_SW_CTL, OUTPUT);
    digitalWrite(RF_SW, HIGH);
    digitalWrite(RF_SW_CTL, HIGH);   //  external antenna

    //uint8_t Result = Radio_Init( RADIO_MODE_RECEIVER, 90, StreamPacketBytes );

    //if (Result == 1) digitalWrite(LED_PIN, LOW);

    pinMode(RADIO_GOOD_PIN, OUTPUT);

    delay(20);

    //setupBareMetalFastTx();

}



#define COMMAND_NONE               0
#define COMMAND_RADIOCHANNEL       65
#define COMMAND_ANTENNA            66
#define COMMAND_MODE               67
#define COMMAND_PACKETLENGTH       70
#define COMMAND_SAMPLERATE         71

void CalculatePacketGoodPinTime()
{
  if (SampleRate == 32000)
  {
    if (StreamPacketBytes == 128) PacketGoodPinTime = 500;
    if (StreamPacketBytes == 64)  PacketGoodPinTime = 250;
    if (StreamPacketBytes == 32)  PacketGoodPinTime = 125;
  }
  if (SampleRate == 44100)
  {
    if (StreamPacketBytes == 128) PacketGoodPinTime = 363;
    if (StreamPacketBytes == 64)  PacketGoodPinTime = 181;
    if (StreamPacketBytes == 32)  PacketGoodPinTime = 91;
  }
}

void processcommand(uint8_t pcommand, uint8_t pvalue) 
{
  if (pcommand == COMMAND_NONE) 
  {
    
  }

  if (pcommand == COMMAND_MODE) 
  {
    RadioMode = pvalue; 
    
    uint8_t Result = Radio_Init( RadioMode, RadioChannel, StreamPacketBytes );
    if (Result == 1) digitalWrite(LED_PIN, LOW);

    //SPI_Slave_Init( StreamPacketBytes + 4 );

    Serial.println( String("mode: ") + String(pvalue) );
  }
  
  if ( pcommand == COMMAND_RADIOCHANNEL ) 
  {
    RadioChannel = pvalue;
    
    uint8_t Result = Radio_Init( RadioMode, RadioChannel, StreamPacketBytes );
    //digitalWrite(LED_PIN, HIGH);
    
    Serial.println( String("channel: ") + String(pvalue) );
  } 

  if ( pcommand == COMMAND_ANTENNA ) 
  {
    RadioChannel = pvalue;

    if ( pvalue == 1 ) digitalWrite(RF_SW_CTL, HIGH);   //  external antenna
    if ( pvalue == 2 ) digitalWrite(RF_SW_CTL, LOW);    //  internal antenna
    
    uint8_t Result = Radio_Init( RadioMode, RadioChannel, StreamPacketBytes );
    //digitalWrite(LED_PIN, HIGH);
    
    Serial.println( String("antenna: ") + String(pvalue) );
  }   

  if ( pcommand == COMMAND_SAMPLERATE ) 
  {
    if ( pvalue == 0 ) SampleRate = 32000;   //  external antenna
    if ( pvalue == 1 ) SampleRate = 44100;    //  internal antenna 

    CalculatePacketGoodPinTime();

    Serial.println( String("samplerate: ") + String(SampleRate) );
  }      

  if ( pcommand == COMMAND_PACKETLENGTH ) 
  {
    StreamPacketBytes = pvalue;

    uint8_t Result = Radio_Init( RadioMode, RadioChannel, StreamPacketBytes );

    SPI_Slave_Init( StreamPacketBytes + 4 );

    CalculatePacketGoodPinTime();

    Serial.println( String("packet: ") + String(StreamPacketBytes) );

    if (pvalue == 32) 
    { 
      setupBareMetalFastTx();
      Serial.println( String("32 byte set") );
    }
    //Serial.println( String("mode: ") + String(RadioMode) );
    //Serial.println( String("channel: ") + String(RadioChannel) );
  }   

}




//uint8_t PacketNumber = 0;
//uint8_t LastPacketNumber = 0;

uint8_t bBuffer[128];

uint32_t mTime = 0;
uint32_t lTime = 0;

uint8_t ledgo = 0;
uint8_t pc = 0;
int32_t sum = 0;
int16_t mcBuffer[64];

uint8_t recflag = 0;


void loop() {
  //digitalWrite(LED_RED, HIGH);  // turn the LED off

  if ( RadioMode == 1 )
  {
  
    if ( SPI_Slave_TransferComplete() == true ) 
    {  
        SPI_Slave_ClearTransferFlag();

        memcpy( &SPIPacketIn, spi_rx_buf, StreamPacketBytes + 4 );
      
        uint8_t paramcommand = SPIPacketIn.ccommand;
        uint8_t paramvalue =   SPIPacketIn.cvalue;

        SPI_Slave_ReArm();      

        processcommand(paramcommand, paramvalue);  
          
    }


    //Serial.print( digitalRead(PIN_D3) );
    //Serial.print( "," );
    //Serial.println( digitalRead(PIN_D0)  );
    

    mTime = micros();
    if ( ( mTime - lTime ) > PacketGoodPinTime )  
    {
      digitalWrite(RADIO_GOOD_PIN, LOW);
    }
    

    if ( Radio_DataReceived() == true ) 
    { 
        //Serial.println(mTime - lTime);
        // digitalWrite(LED_RED, LOW);   // turn the LED off by making the voltage LOW
        //Serial.println(20); 

        SPIPacketOut.packetnumber = Radio_ReadBytesInBufferReturnPacketNumber(bBuffer, StreamPacketBytes);

        memcpy(&SPIPacketOut.stream, bBuffer, StreamPacketBytes);
      
        SPIPacketOut.ccommand     = 0;
        SPIPacketOut.cvalue       = 0;
      
        memcpy(spi_tx_buf, &SPIPacketOut, StreamPacketBytes + 4);

        digitalWrite(RADIO_GOOD_PIN, HIGH);

        //Serial.println( mTime - lTime );

        lTime = mTime;

        memcpy( mcBuffer, bBuffer, StreamPacketBytes );
        sum = 0;
        for (int i = 0; i < ( StreamPacketBytes >> 1 ); i++) 
        {
            sum = sum + mcBuffer[i];
        }
        //Serial.println( sum / ( StreamPacketBytes >> 1 ) );

        ledgo = 0;
        for (uint8_t i = 0; i < ( StreamPacketBytes >> 1 ) ; i++)
        {
          if ( mcBuffer[i] > 5000 ) ledgo = 1; 
        }

        if ( ledgo == 1)  digitalWrite(LED_PIN, LOW);
        if ( ledgo == 0)  digitalWrite(LED_PIN, HIGH);
        
        
    }
  }


  if ( RadioMode == 0 )
  {
    //Serial.print( digitalRead(PIN_D3) );
    //Serial.print( "," );
    //Serial.println( digitalRead(PIN_D10)  );

    if ( SPI_Slave_TransferComplete() == true ) 
    {  
      SPI_Slave_ClearTransferFlag();

      memcpy( &SPIPacketIn, spi_rx_buf, StreamPacketBytes + 4 );

      uint8_t paramcommand = SPIPacketIn.ccommand;
      uint8_t paramvalue =   SPIPacketIn.cvalue;

      SPI_Slave_ReArm(); 

      processcommand(paramcommand, paramvalue);    

      delayMicroseconds(20);

      if (paramcommand == 0) recflag = 1;

      
    };



    if ( recflag == 1 )
    {
      memcpy( bBuffer, &SPIPacketIn.stream, StreamPacketBytes );

      recflag = 0;
      
      Radio_WriteBytesFromBufferWithPacketNumber( bBuffer, StreamPacketBytes, (int16_t)SPIPacketIn.packetnumber );  

      mTime = micros();
      //Serial.println( mTime - lTime );
      lTime = mTime;

      memcpy( mcBuffer, bBuffer, StreamPacketBytes );
      sum = 0;
      for (int i = 0; i < ( StreamPacketBytes >> 1 ); i++) 
      {
          sum = sum + mcBuffer[i];
      }
      //Serial.println(sum / ( StreamPacketBytes >> 1 ) );

      ledgo = 0;
      for (uint8_t i = 0; i < ( StreamPacketBytes >> 1 ) ; i++)
      {
        if ( mcBuffer[i] > 5000 ) ledgo = 1; 
      }

      if ( ledgo == 1)  digitalWrite(LED_PIN, LOW);
      if ( ledgo == 0)  digitalWrite(LED_PIN, HIGH);  

    }
  }
}