
#define PACKET_LENGTH_SAMPLES 16
#define PACKET_LENGTH_BYTES   32


#include "BoardPin.h" 
#include "Input.h"
#include "Output.h" 

//#include "Stream_I2S_New.h"   
#include "Stream_I2S.h"
#include "Radio_nRF24.h"

#include "Stream_Process.h"


uint8_t bBuffer[PACKET_LENGTH_SAMPLES * 2];

int16_t mBuffer[PACKET_LENGTH_SAMPLES];
int16_t mBuffer2[PACKET_LENGTH_SAMPLES];

int32_t exmBuffer[32];



// https://github.com/plerup/espsoftwareserial/
#include <SoftwareSerial.h>

SoftwareSerial MySerial2(UART_RX_2, UART_TX_2, true);

uint8_t radiosuccess2;


/*
// the 7 channels for the messages to be received and to be sent out
#define MCHAN_SET_LEVEL_VOLUME1  2
#define MCHAN_SET_LEVEL_VOLUME2  3
#define MCHAN_SET_RELAY          1

//void MessageReceivedValue(int Message, int Value);
//#include "Messenger.h"

#include "Network.h"

TaskHandle_t Task1;

uint8_t count = 0;
uint8_t connected = 0;

int Messenger_Connected = 0;

void Task1code( void * pvParameters) {
  for(;;) 
  {
    //digitalWrite(PIN_TASK_LED, HIGH);
    //delay(1000);
    //digitalWrite(PIN_TASK_LED, LOW);

    //SevenSegmentDisplay_ShowNumber(count);

    delay(100);
  }
}
*/


uint8_t SignalInput = 0;

void setup() {

  Serial.begin(230400);
  delay(100);

  //SevenSegmentDisplay_Init(PIN_LEDDISPLAY_CLOCK, PIN_LEDDISPLAY_DATA);

  //int intSuccess = Messenger_Init();          // connect to wifi
  //Messenger_SetPathAndDirection(MCHAN_SET_LEVEL_VOLUME1, "A/Volume/Setting",       2);
  //Messenger_Start();  // start the messenger
  //Messenger_Connected = Network_Connect("ESP32_MQTT_AP", "");

  //pinMode(PIN_TASK_LED, OUTPUT);

  // https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
  /*
  xTaskCreatePinnedToCore(
      Task1code, // Function to implement the task 
      "Task1",   // Name of the task 
      20000,     // Stack size in words 
      NULL,      // Task input parameter 
      0,         // Priority of the task 
      &Task1,    // Task handle
      0);        // Core where the task should run
  */

  SignalInput = 1;

  Radio_Init_Dual(0);
  delay(100);

  Serial.println("Starting Receivers");

  radiosuccess2 = Radio2_Start(RADIO_MODE_RECEIVER);
  
  uint8_t radiosuccess = Radio1_Start(RADIO_MODE_RECEIVER);
  if (radiosuccess == 0) 
  { 
    delay(100);
    radiosuccess = Radio1_Start(RADIO_MODE_RECEIVER);
  }
  delay(100);
  if (radiosuccess2 == 0) 
  { 
    radiosuccess2 = Radio2_Start(RADIO_MODE_RECEIVER);
  }


  //MySerial.begin(2000000, SERIAL_8N1, UART_RX, UART_TX);
  MySerial2.begin(1200);
  delay(200);

  AudioMode = AUDIO_MODE_SAMPLER_PLAYER;
  samplebuffer_count = 4;
  samplebuffer_length = 16;
  samplerate = 32000;
  audiochannelcount = 2;
  expandchannels = 0;
  volumecontrol = 1; 
  I2S_Start();
  delay(100);

  Input_Init();
  Output_Init();

  if (radiosuccess == 1 && radiosuccess2 == 1)
  {
    Output_SetValue(OCHAN_ERROR, 0);
    Output_SetValue(OCHAN_RADIOCHANNEL1, RadioChannelIndex1);
    Output_ShowValue(OCHAN_RADIOCHANNEL2, RadioChannelIndex2);
  }
  else
  {
    Output_ShowValue(OCHAN_ERROR, 1);
  }


  Volume = 50;
  
}






int32_t TimeReceived = 0;
int32_t LastTimeReceived1 = 0;
int32_t LastTimeReceived2 = 0;

uint8_t PacketReceived1 = 0;
uint8_t PacketReceived2 = 0;

uint8_t ActiveModule = 1;

uint8_t ActiveModuleHasChanged1 = 0;
uint8_t ActiveModuleHasChanged2 = 0;


uint16_t TimeOut = 300;

uint8_t PacketNotReceivedCount = 0;


int8_t ChangeInput(uint8_t NewInput)
{
  int8_t RB = -1;
  
  if (NewInput == 1)
  {
    SignalInput = NewInput;

    AudioMode = AUDIO_MODE_SAMPLER_PLAYER;
    samplebuffer_count = 4;
    samplebuffer_length = 16;
    samplerate = 32000;
    audiochannelcount = 2;
    expandchannels = 0;
    volumecontrol = 1; 
    I2S_Restart();

    RB = (int8_t)NewInput;
  }

  if (NewInput == 2)
  {
    SignalInput = NewInput;

      // --- I2S (Audio Output) Setup ---
    AudioMode = AUDIO_MODE_SAMPLER_PLAYER;
    samplebuffer_count = 2;
    samplebuffer_length = 8;
    samplerate = 32000;        // Standard CD audio sample rate
    audiochannelcount = 2;     // Stereo
    expandchannels = 0;
    volumecontrol = 1; 
    I2S_Restart();

    RB = (int8_t)NewInput;
  }

  return RB;
}

int8_t NextRadioChannel(uint8_t RadioNumber)
{
  int8_t RB = -1;
  
  if (RadioNumber == 1)
  {
    RadioChannelIndex1++;
    if (RadioChannelIndex1 >= RADIOCHANNEL_INDEXMAX) RadioChannelIndex1 = 0;  // RadioChannelIndex from 0 to 4

    Radio1_SetChannel(RadioChannelIndex1);
    Radio1_Start(RADIO_MODE_RECEIVER);
    delay(10);

    RB = RadioChannelIndex1;
    //Output_ShowValue(OCHAN_RADIOCHANNEL1, RadioChannelIndex1);
  }

  if (RadioNumber == 2)
  {
    RadioChannelIndex2++;
    if (RadioChannelIndex2 >= RADIOCHANNEL_INDEXMAX) RadioChannelIndex2 = 0;  // RadioChannelIndex from 0 to 4

    Radio2_SetChannel(RadioChannelIndex2);
    Radio2_Start(RADIO_MODE_RECEIVER);
    delay(10);

    RB = RadioChannelIndex2;
    //Output_ShowValue(OCHAN_RADIOCHANNEL2, RadioChannelIndex2);
  }

  PacketReceived1 = 0;
  PacketReceived2 = 0;

  return RB;
}



void loop() { 

  if (SignalInput == 1)
  {
    TimeReceived = micros(); // current time in microseconds

    if( Radio1_DataReceived() )
    {
      PacketReceived1 = 1;   // flag: packet from radio module 1 
      //Serial.println(TimeReceived - LastTimeReceived1);
      LastTimeReceived1 = TimeReceived;
    }

    if( Radio2_DataReceived() )
    {
      PacketReceived2 = 1;  // flag: packet from radio module 2  
      LastTimeReceived2 = TimeReceived;
    }

    if ( ActiveModule == 1 )
    {
      if ( TimeReceived - LastTimeReceived1 > TimeOut ) // packet not arrived in time
      {
        if (ActiveModuleHasChanged1 == 1)  // have we just changed to module 1?
        {
          I2S_Restart();
        }
        ActiveModule = 2;  // change to module 2
        ActiveModuleHasChanged2 = 1; // flag: we just changed to module 2
      }
      else
      {
        ActiveModuleHasChanged1 = 0; // clear flag that we just changed to module 1
      }
    }
    else
    {
      if ( TimeReceived - LastTimeReceived2 > TimeOut ) // packet not arrived in time
      {
        if (ActiveModuleHasChanged2 == 1)  // have we just changed to module 2?
        {
          I2S_Restart();
        }
        ActiveModule = 1;  // change to module 1
        ActiveModuleHasChanged1 = 1; // flag: we just changed to module 1
      }
      else
      {
        ActiveModuleHasChanged2 = 0; // clear flag that we just changed to module 2
      }
    }

    Output_ShowValue(OCHAN_ACTIVE_MODULE, ActiveModule);  // visualize active module (will switch LED on or off)

    if (PacketReceived1 == 1 && ActiveModule == 1 )  // packet received by module 1
    {         
        Radio1_ReadSamplesInBuffer(mBuffer, PACKET_LENGTH_SAMPLES);
        //I2S_WriteSamplesFromBuffer(mBuffer, PACKET_LENGTH_SAMPLES);

        Process_Process(mBuffer, exmBuffer, 16);
        I2S_WriteSamplesFromBuffer32(exmBuffer, 16);
        
        PacketReceived1 = 0; // clear flag
        PacketReceived2 = 0;
    }

    if (PacketReceived2 == 1 && ActiveModule == 2 ) // // packet received by module 2
    {         
        Radio2_ReadSamplesInBuffer(mBuffer2, PACKET_LENGTH_SAMPLES);
        //I2S_WriteSamplesFromBuffer(mBuffer2, PACKET_LENGTH_SAMPLES);
        
        Process_Process(mBuffer2, exmBuffer, 16);
        I2S_WriteSamplesFromBuffer32(exmBuffer, 16);

        PacketReceived2 = 0; // clear flag
        PacketReceived1 = 0;
    }
  }

  if (SignalInput == 2)
  {
    I2S_2_ReadBytesInBuffer(bBuffer, 32);

    memcpy(&mBuffer, &bBuffer, 32);

    Process_Process(mBuffer, exmBuffer, 16);

    I2S_WriteSamplesFromBuffer32(exmBuffer, 16);

  }

    // --- Check for serial commands
  if (MySerial2.available()) 
  {
    
    uint8_t rBuf[3];
    int8_t Returnbyte = 0;

    MySerial2.readBytes(rBuf, 3);

    command =   rBuf[0];
    incoming1 = rBuf[1];
    incoming2 = rBuf[2];

    Serial.println(rBuf[0]);
    
    if (command != 73 && command != 82 && command != 83) Process_SetParameters();
    if (command == 73) Returnbyte = ChangeInput(incoming2 - 64);
    if (command == 82) Returnbyte = NextRadioChannel(1);
    if (command == 83) Returnbyte = NextRadioChannel(2);

    MySerial2.write(uint8_t(Returnbyte + 64));
  }


  // --- Check for serial commands
  if (Serial.available()) 
  {
    
    uint8_t rBuf[3];
    int8_t Returnbyte = 0;

    Serial.readBytes(rBuf, 3);

    command =   rBuf[0];
    incoming1 = rBuf[1];
    incoming2 = rBuf[2];

    //Serial.println(rBuf[0]);
    
    if (command != 73 && command != 82 && command != 83) Process_SetParameters();
    if (command == 73) Returnbyte = ChangeInput(incoming1 - 64);
    if (command == 82) Returnbyte = NextRadioChannel(1);
    if (command == 83) Returnbyte = NextRadioChannel(2);
    
    Serial.write(uint8_t(Returnbyte + 64));
  }

  Input_Loop();

  int IChannelValueWasChanged = Input_GetChannelOfChangedValue();  // there was a message received at one channel
  
  if (IChannelValueWasChanged > -1) 
  {
    if (IChannelValueWasChanged == 1) // short press
    {
      NextRadioChannel(1);
    }

    if (IChannelValueWasChanged == 0) // long press
    {
      NextRadioChannel(2);
    }
  } 
  
 
}