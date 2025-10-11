#include <Arduino.h>
//#include <WiFi.h>

#include "BoardPin.h"          // this helps to switch from ESP32 XIAO S3 to C3



// the 7 channels for the messages to be received and to be sent out
#define MCHAN_SET_LEVEL_VOLUME1  2
#define MCHAN_SET_LEVEL_VOLUME2  3
#define MCHAN_SET_RELAY          1

#define MCHAN_LEVEL_VOLUME1     4
#define MCHAN_LEVEL_VOLUME2     5
#define MCHAN_VOLTAGE           6
#define MCHAN_TEMP              7

#define MCHAN_SET_LEVEL_LOWPASS      8
#define MCHAN_SET_LEVEL_HIGHPASS     9
#define MCHAN_SET_PARAMEQ1_FREQUENCY  10
#define MCHAN_SET_PARAMEQ2_FREQUENCY  14
#define MCHAN_SET_INPUT              11
#define MCHAN_SETNEXT_RADIO1         12
#define MCHAN_SETNEXT_RADIO2         13




// include all the interfaces "CAISOM"
#include "Config.h"
#include "Input.h"


#include "Output.h"


void MessageReceivedValue(int Message, int Value);
#include "Messenger.h"


// the 4 operating values for the main program (global variables)
#define PCHAN_LEVEL_VOLUME       0
#define PCHAN_LEVEL_VOLUME1      0
#define PCHAN_LEVEL_VOLUME2      1
#define PCHAN_SET_LEVEL_VOLUME1     2
#define PCHAN_SET_LEVEL_VOLUME2     3

#define PCHAN_SET_LEVEL_LOWPASS     4
#define PCHAN_SET_LEVEL_HIGHPASS    5
#define PCHAN_SET_PARAMEQ1_FREQUENCY 6
#define PCHAN_SET_PARAMEQ2_FREQUENCY 10
#define PCHAN_SET_INPUT             7
#define PCHAN_SET_RADIO1            8
#define PCHAN_SET_RADIO2            9

int PValue[16];

#include "SerialCED.h"

/*
#include <HardwareSerial.h>
HardwareSerial MySerial(1); // define a Serial for UART1
*/

#include <SoftwareSerial.h>
SoftwareSerial MySerial(UART_RX, UART_TX);


void setup() {
 
  Serial.begin(115200);

  Serial.println("Hello I am your AVS Node!");

  //MySerial.begin(9600, SERIAL_8N1, UART_RX, UART_TX);
  MySerial.begin(1200);
  delay(200);

  // initialization of the operation variables
  PValue[PCHAN_LEVEL_VOLUME1] = 5;

  PValue[ PCHAN_SET_RADIO1 ] = 0;
  PValue[ PCHAN_SET_RADIO2 ] = 0;


  Output_Init();  // prepare the 7 segment display

  if (Config_Init() == -1) Output_SetValue(OCHAN_ERROR, ERROR_CONFIG);    // get the values of the configuration device (DIP Switch) 

  Output_SetValue(OCHAN_DEVICENUMBER, Config_GetValue(CCHAN_DEVICENUMBER));        // let the output interface know about the device number of the station
   

  if (Config_GetValue(CCHAN_DONOTCONNECT_WIFI) == 0)     // we want to connect to wifi
  {
    int intSuccess = Messenger_Init();          // connect to wifi

    if (intSuccess == -1) 
    {
      Output_SetValue(OCHAN_WIFISTATUS, WIFI_NOTCONNECTED);    // let the output interface know that connection was not successful
    }
    else
    {
      Output_SetValue(OCHAN_WIFISTATUS, WIFI_CONNECTED);    // let the output interface know that connection was successful
    }
  }
  else
  {
    Output_SetValue(OCHAN_WIFISTATUS, WIFI_NOTCONNECTED);  // let the output interface know that there is no wifi connection
  }


  Input_Init();  // start the IR receiver

  Messenger_SetPathAndDirection(MCHAN_LEVEL_VOLUME1    , "A/Volume/Actual",        1);
  Messenger_SetPathAndDirection(MCHAN_LEVEL_VOLUME2    , "B/Volume/Actual",        1);
  Messenger_SetPathAndDirection(MCHAN_SET_LEVEL_VOLUME1, "A/Volume/Setting",       2);
  Messenger_SetPathAndDirection(MCHAN_SET_LEVEL_VOLUME2, "B/Volume/Setting",       2);
  Messenger_SetPathAndDirection(MCHAN_SET_LEVEL_LOWPASS,  "/LowPass/Setting",      2);
  Messenger_SetPathAndDirection(MCHAN_SET_LEVEL_HIGHPASS, "/HighPass/Setting",     2);
  Messenger_SetPathAndDirection(MCHAN_SET_PARAMEQ1_FREQUENCY, "/ParamEQ/1/Frequency/Setting", 2);
  Messenger_SetPathAndDirection(MCHAN_SET_PARAMEQ2_FREQUENCY, "/ParamEQ/2/Frequency/Setting", 2);
  Messenger_SetPathAndDirection(MCHAN_SET_INPUT, "/Input/Setting", 2);
  Messenger_SetPathAndDirection(MCHAN_SETNEXT_RADIO1, "/Radio/1/Next", 2);
  Messenger_SetPathAndDirection(MCHAN_SETNEXT_RADIO2, "/Radio/2/Next", 2);


  if (Config_GetValue(CCHAN_DONOTCONNECT_WIFI) == 0)    // if connection was wanted
  {
    Messenger_SetStationNumber(Config_GetValue(CCHAN_DEVICENUMBER));  // let the messenger interface know about the device number

    Messenger_Start();  // start the messenger

    Messenger_SetValue(MCHAN_LEVEL_VOLUME1, PValue[PCHAN_LEVEL_VOLUME1] / 10);  // 
    Messenger_SetValue(MCHAN_LEVEL_VOLUME2, PValue[PCHAN_LEVEL_VOLUME2] / 10);

    if (Config_GetValue(CCHAN_DONOTSEND_WIFI) == 0) Messenger_SendValues();  // send the messages
  }

  Output_SetValue(OCHAN_LEVEL_VOLUME1, PValue[PCHAN_LEVEL_VOLUME1] / 10);    // let the output interface know about motor pot level
  Output_SetValue(OCHAN_LEVEL_VOLUME2, PValue[PCHAN_LEVEL_VOLUME2] / 10); 
  
  Output_Refresh(REFRESHTYPE_FULL);  // and show the values

  SerialCED_Init();
}



// limits an operation variable to a certain range between min and max
void Limit(int PChannel, int Min, int Max)
{
  if (PValue[PChannel] < Min)   PValue[PChannel] = Min;
  if (PValue[PChannel] > Max)   PValue[PChannel] = Max;
}



void SendAudioRadioParameter(uint8_t Command, uint16_t ParameterValue)
{
  uint8_t sbuf[3];

  sbuf[0] = Command;
  sbuf[1] = 64 + (ParameterValue / 16) ;
  sbuf[2] = 64 + (ParameterValue & 0x0F);

  MySerial.write(sbuf, 3);
}



void Regulate(uint8_t audiochannelnumber)
{
  
  // TODO: DUAL!!!!
  
  uint8_t sbuf[3];
  uint16_t VolumeSend = PValue[ PCHAN_SET_LEVEL_VOLUME1 ] / 10;

  sbuf[0] = 86;
  sbuf[1] = 64 + (VolumeSend / 16) ;
  sbuf[2] = 64 + (VolumeSend & 0x0F);

  MySerial.write(sbuf, 3);

  PValue[ PCHAN_LEVEL_VOLUME1 ] = PValue[ PCHAN_SET_LEVEL_VOLUME1 ];
  Output_ShowValue(OCHAN_LEVEL_VOLUME1, PValue[ PCHAN_LEVEL_VOLUME1 ] / 10);
  Output_ShowEvent(EVENT_MESSAGE_RECEIVED, 700); 
}




long PeriodMessageVolume = 1000;   // in ms
long LastTimeMessageVolume = 0;

uint8_t SerialReplyExpected = 0;

void loop() {
  
  //Regulate(0);
  
  //if (Config_GetValue(CCHAN_DOUBLESTATION) == 1) Regulate(1);  // regulator loop for the second motor pot (if there is one)

  if ((millis() - LastTimeMessageVolume) > PeriodMessageVolume)   // each half second
  {
    LastTimeMessageVolume = millis();

    if (Config_GetValue(CCHAN_DONOTCONNECT_WIFI) == 0)     // if we wanted wifi connection
    {
      Messenger_SetValue(MCHAN_LEVEL_VOLUME1,  PValue[PCHAN_LEVEL_VOLUME1] / 10);   // send out the value of the motor pot
    
      if (Config_GetValue(CCHAN_DOUBLESTATION) == 1)
      {         
        Messenger_SetValue(MCHAN_LEVEL_VOLUME2,  PValue[PCHAN_LEVEL_VOLUME2] / 10);    // send out the value of the second motor pot 
      }  
        
      if (Config_GetValue(CCHAN_DONOTSEND_WIFI) == 0) Messenger_SendValues(); // send the messages

      Messenger_Loop();     // look if MQTT messages were received 

      int MChannelValueWasReceived = Messenger_GetChannelOfReceivedValue();  // there was a message received at one channel
      
      if (MChannelValueWasReceived > -1) 
      {
        
        switch (MChannelValueWasReceived) {   
    
          case MCHAN_SET_LEVEL_VOLUME1:
            PValue[PCHAN_SET_LEVEL_VOLUME1] = Messenger_GetValue(MChannelValueWasReceived) * 10;  // message to control the motor pot 1
            Limit(PCHAN_SET_LEVEL_VOLUME1, 0, 1400);
            Regulate(0);
            break;

          case MCHAN_SET_LEVEL_VOLUME2:
            PValue[PCHAN_SET_LEVEL_VOLUME2] = Messenger_GetValue(MChannelValueWasReceived) * 10;  // message to control the motor pot 2
            Limit(PCHAN_SET_LEVEL_VOLUME2, 0, 1400);
            if (Config_GetValue(CCHAN_DOUBLESTATION) == 1) Regulate(1);  // regulator loop for the second motor pot (if there is one)
            break;

          case MCHAN_SET_LEVEL_LOWPASS:
            PValue[PCHAN_SET_LEVEL_LOWPASS] = Messenger_GetValue(MChannelValueWasReceived);  // message 
            SendAudioRadioParameter(76, PValue[PCHAN_SET_LEVEL_LOWPASS]);
            Output_SetValue(OCHAN_AUDIO_PARAM, PValue[ PCHAN_SET_LEVEL_LOWPASS ] );
            Output_ShowEvent(EVENT_SHOW_AUDIO_PARAM, 4000);
            break;

          case MCHAN_SET_LEVEL_HIGHPASS:
            PValue[PCHAN_SET_LEVEL_HIGHPASS] = Messenger_GetValue(MChannelValueWasReceived);  // message 
            SendAudioRadioParameter(72, PValue[PCHAN_SET_LEVEL_HIGHPASS]);
            Output_SetValue(OCHAN_AUDIO_PARAM, PValue[ PCHAN_SET_LEVEL_HIGHPASS ] );
            Output_ShowEvent(EVENT_SHOW_AUDIO_PARAM, 4000);
            break;

          case MCHAN_SET_PARAMEQ1_FREQUENCY:
            PValue[PCHAN_SET_PARAMEQ1_FREQUENCY] = Messenger_GetValue(MChannelValueWasReceived);  // message 
            SendAudioRadioParameter(80, PValue[ PCHAN_SET_PARAMEQ1_FREQUENCY ] / 40);
            Output_SetValue(OCHAN_AUDIO_PARAM, PValue[ PCHAN_SET_PARAMEQ1_FREQUENCY ] );
            Output_ShowEvent(EVENT_SHOW_AUDIO_PARAM, 4000);
            break;

          case MCHAN_SET_PARAMEQ2_FREQUENCY:
            PValue[PCHAN_SET_PARAMEQ2_FREQUENCY] = Messenger_GetValue(MChannelValueWasReceived);  // message 
            SendAudioRadioParameter(81, PValue[ PCHAN_SET_PARAMEQ2_FREQUENCY ] / 40);
            Output_SetValue(OCHAN_AUDIO_PARAM, PValue[ PCHAN_SET_PARAMEQ2_FREQUENCY ] );
            Output_ShowEvent(EVENT_SHOW_AUDIO_PARAM, 4000);
            break;

          case MCHAN_SET_INPUT:
            PValue[PCHAN_SET_INPUT] = Messenger_GetValue(MChannelValueWasReceived);  // message to control the motor pot 1
            SendAudioRadioParameter(73, PValue[ PCHAN_SET_INPUT ]);
            Output_SetValue(OCHAN_INPUT, PValue[ PCHAN_SET_INPUT ] );
            Output_ShowEvent(EVENT_SHOW_INPUT, 2000);
            break;

          case MCHAN_SETNEXT_RADIO1:
            PValue[ PCHAN_SET_RADIO1 ]++;
            if (PValue[ PCHAN_SET_RADIO1 ] > 5) PValue[ PCHAN_SET_RADIO1 ] = 0;
            SendAudioRadioParameter(82, PValue[ PCHAN_SET_RADIO1 ]);
            Output_SetValue(OCHAN_RADIO1_INDEX, PValue[ PCHAN_SET_RADIO1 ] );
            Output_ShowEvent(EVENT_SHOW_RADIO1, 2000);
            break;

          case MCHAN_SETNEXT_RADIO2:
            PValue[ PCHAN_SET_RADIO2 ]++;
            if (PValue[ PCHAN_SET_RADIO2 ] > 5) PValue[ PCHAN_SET_RADIO2 ] = 0;
            SendAudioRadioParameter(83, PValue[ PCHAN_SET_RADIO2 ]);
            Output_SetValue(OCHAN_RADIO2_INDEX, PValue[ PCHAN_SET_RADIO2 ] );
            Output_ShowEvent(EVENT_SHOW_RADIO2, 2000);
            break;
        
       }

      }

    }
    
  }


  if (MySerial.available() )
  {
    uint8_t  Returnbyte = MySerial.read();

    if (SerialReplyExpected == 1)
    {
      Output_ShowValue(OCHAN_LEVEL_VOLUME1, Returnbyte - 64);
      Output_ShowEvent(EVENT_MESSAGE_RECEIVED, 700);

      Serial.println(Returnbyte - 64);

      SerialReplyExpected = 0;
    }
  }

  // --- Check for serial commands
  if (Serial.available()) 
  {
    uint8_t rBuf[3];

    Serial.readBytes(rBuf, 3);

    MySerial.write(rBuf, 3);

    SerialReplyExpected = 1;
  }


  Input_Loop();  // look if IR button presses were received
  
  int IChannelValueWasChanged = Input_GetChannelOfChangedValue();  // there was a message received at one channel
  
  if (IChannelValueWasChanged > -1) 
  {
    
    switch (IChannelValueWasChanged) {
    

    }

    Output_ShowEvent(EVENT_MESSAGE_RECEIVED, 700);        // show that a button was pressed (in that case the 7 segment display) 
  
  }

  Output_EventLoop();  // Refresh the 7-Segment display with new values

  //SerialCED_Loop();
    
  delay(10); 
}
