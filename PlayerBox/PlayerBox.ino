/*
MIT License

Copyright (c) 2025, 2026 Jens Nickel, Elektor Labs 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/



#include <Arduino.h>

#define PACKET_LENGTH_SAMPLES 16
#define PACKET_LENGTH_BYTES   32


#define SIGNAL_AIR           1
#define SIGNAL_ANALOG        2
#define SIGNAL_DIGITAL       3
#define SIGNAL_TEST_MONO     4
#define SIGNAL_TEST_STEREO   5


#include "BoardPin.h"        // Board-specific pin definitions
#include "Config.h"          // Module for configuration values (DIP switch and non-volatile memory) 

#include "Radio_nRF24.h"     // radio library
#include "Stream_I2S.h"      // I2S audio output functions

#include "Clock_I2S.h"       // library for configuring clock module
#include "Clock_Sync.h"      // library for tuning mechanism (received packet count vs. clock speed)

#include "Stream_Process.h"  // library for Audio processing (Volume, Filters...)
#include "Stream_RS485.h"    // library for audio stream via RS485
#include "Stream_TestSignals.h" // library for test signal audio stream   

#include "Output.h"           // module for output functions (display, LED) 
#include "Input.h"            // module for input functions (IR receiver, buttons)

#include "ESP_NOW_Control.h"  // library for remote control via ESP-NOW
#include "Serial_Control.h"   // library for remote control via Serial interface

#include "CoreZero.h"         // the functions which run on CoreZero are integrated here


uint8_t DeviceAddress = 1;    // Device Address, will be configured by DIP switch
uint8_t Tuning_Live = 1;      // Tuning in action or not


// === INITIALIZATION (Runs Once at Startup) ===
void setup() 
{
  
  Serial.begin(115200);       // start serial interface
  delay(200);

  Serial.println("Audioboard starting");

  ESP_NOW_Connect(ESP_NOW_RECEIVER, 1);   // start ESP-NOW connection with this board as receiver
  
  RS485_Start(UART_RX, UART_TX);      // start the hardware serial interface for fast audio transmission (3 Mbit/s) 

  Radio_Init_Dual(500);          // initialize both nRF24 modules, with a timeout of 500 ms for switching the active module
  delay(100);
 
  Radio_Start_Dual(pCE, pCS, pCE2, pCS2);            // start both nRF24 modules

  Wire.begin(I2C_SDA, I2C_SCL, 400000);   // start I2C bus, you need that command once in the code

  Output_Init();       // initialize the software modules
  Input_Init();
  Config_Init();

  DeviceAddress = Config_GetValue(CCHAN_DEVICE_ADDRESS);  //  can be 1 to 4, and each device can output 2 audio channels, for 8 loudspeakers in total
  // address 1 and 3 will output music generated from left input audio channel, 2 and 4 is dedicated to output the right input channel
  
  CurrentCorrection =  Config_GetValue(CCHAN_CORRECTION);   // get the correction for the clock in ppb from the non-volatile memory
  
  if (CurrentCorrection < -200000 || CurrentCorrection > 200000)   // avoid crazy random values
  {
	  CurrentCorrection = 0;
  }

 
  SignalInput = SIGNAL_AIR;        // Default: wireless transmission
  //SignalInput = SIGNAL_DIGITAL;  // RS485 can be selected here
  
 
  Process_SetParameters(COMMAND_VOLUME, AUDIOOUTPUT_CHANNEL_BOTH, 60);
  ESP_NOW_Send_Command( DeviceAddress, 100 + COMMAND_VOLUME, 0, 60 );


  if ( Config_GetValue(CCHAN_DEVICE_SUBSAT) == 1 ) 
  {
    if ( (DeviceAddress & 0x01) == 0 ) Process_SetParameters(COMMAND_ROUTING, 0, ROUTING_EXPAND_RIGHT);
    if ( (DeviceAddress & 0x01) == 1 ) Process_SetParameters(COMMAND_ROUTING, 0, ROUTING_EXPAND_LEFT);
    
    Process_SetParameters(COMMAND_BASSBALANCE,    AUDIOOUTPUT_CHANNEL_SUB, 100);
    Process_SetParameters(COMMAND_LOWPASS,        AUDIOOUTPUT_CHANNEL_SUB, 100);
  }

  Output_SetValue( OCHAN_INPUT, SignalInput );
  Output_SetValue( OCHAN_DEVICENUMBER, DeviceAddress );
  Output_ShowValue( OCHAN_LEVEL_VOLUME, 60 );    // let the output interface know about the default volume

    // --- I2S (Audio Output) Setup ---
  AudioMode = AUDIO_MODE_PLAYER;
  samplebuffer_count = 4;
  samplebuffer_length = 8;
  samplerate = 32000;        // audio sample rate
  
  audiochannelcount = 2;     // Stereo
  expandchannels = 0;        
  externalclock = 1;         // we use the external clock, so ESP32 in slave mode
  volumecontrol = 1;         // we use volume control, so expanding 16 bit to 32 bit so no resolution is lost
  I2S_Start();
  delay(200);

  I2S_Clock_Init(11, CurrentCorrection);   // start the clock with the current correction

  Sync_Init(8);    // Initialize Live-Tuning with a GateTimeFactor of 8 = 1250000 packets
  
  StartUARTTask();    // start the task for the UART/RS485 receiving of packets in the Core0 module
  StartAudioTask();   // start the task for the audio (receiving packets, processing, output to the DAC) in the Core0 module
 
}




int32_t LimitValue(int32_t CurrentValue, int32_t LowerLimit, int32_t UpperLimit)
{
  int32_t ResultValue = CurrentValue;
  if (ResultValue < LowerLimit) ResultValue = LowerLimit;
  if (ResultValue > UpperLimit) ResultValue = UpperLimit;
  return ResultValue;
}


int32_t CurrentMaxValueInputLeft;
int32_t CurrentMaxValueInputRight;

#define COMMAND_SIGNALINPUT  73   // I
#define COMMAND_DISPLAYMODE  89   // Y



void CommandAndDisplay(uint8_t CommandReceived, uint8_t AudioOutputChannelReceived, int32_t ValueReceived)
{
    int32_t currentValue;
    
    switch(CommandReceived)
    {
        case COMMAND_VOLUME:
            currentValue = Process_SetParameters(CommandReceived, 0, ValueReceived);
            Output_ShowValue( OCHAN_LEVEL_VOLUME, currentValue ); 
            Output_ShowEvent( EVENT_MESSAGE_RECEIVED, 700 );  

            delay(DeviceAddress * 50 + 70);
            ESP_NOW_Send_Command( DeviceAddress, 100 + COMMAND_VOLUME, 0, currentValue );
            
          break;

        case COMMAND_BASSBALANCE:
            currentValue = Process_SetParameters(COMMAND_BASSBALANCE, AUDIOOUTPUT_CHANNEL_BOTH, ValueReceived);
            Output_SetValue( OCHAN_BASSBALANCE, currentValue );
            Output_ShowEvent( EVENT_SHOW_BASSBALANCE, 1000 );   

            delay(DeviceAddress * 50 + 70);
            ESP_NOW_Send_Command( 0, 100 + COMMAND_BASSBALANCE, 0, currentValue );
            
          break;

        case COMMAND_LOWPASS:
            if (Config_GetValue(CCHAN_DEVICE_SUBSAT) == 1) 
            {
              AudioOutputChannelReceived = AUDIOOUTPUT_CHANNEL_SUB;
              ValueReceived = LimitValue(ValueReceived, 20, 200);
            }  
            else
            {
              ValueReceived = LimitValue(ValueReceived, 0, 15000);
            }           
            currentValue = Process_SetParameters(CommandReceived, AudioOutputChannelReceived, ValueReceived);
            Output_SetValue( OCHAN_LOWPASS, currentValue );
            Output_ShowEvent( EVENT_SHOW_LOWPASS, 2000 );

            delay(DeviceAddress * 50 + 70);
            ESP_NOW_Send_Command( 0, 100 + COMMAND_LOWPASS, 0, currentValue ); 
            
          break;

        case COMMAND_HIGHPASS:
            if (Config_GetValue(CCHAN_DEVICE_SUBSAT) == 1) AudioOutputChannelReceived = AUDIOOUTPUT_CHANNEL_SAT;
            ValueReceived = LimitValue(ValueReceived, 0, 15000);
            currentValue = Process_SetParameters(CommandReceived, AudioOutputChannelReceived, ValueReceived);
            Output_SetValue( OCHAN_HIGHPASS, currentValue );
            Output_ShowEvent( EVENT_SHOW_HIGHPASS, 2000 ); 

            delay(DeviceAddress * 50 + 70);
            ESP_NOW_Send_Command( 0, 100 + COMMAND_HIGHPASS, 0, currentValue );
          break;

        case COMMAND_PARAMETRIC:
            if (Config_GetValue(CCHAN_DEVICE_SUBSAT) == 1) AudioOutputChannelReceived = AUDIOOUTPUT_CHANNEL_SAT;
            ValueReceived = LimitValue(ValueReceived, 0, 15000);
            currentValue = Process_SetParameters(CommandReceived, AudioOutputChannelReceived, ValueReceived);
            Output_SetValue( OCHAN_PARAMETRIC, currentValue );
            Output_ShowEvent( EVENT_SHOW_PARAMETRIC, 2000 ); 
          break;

        case COMMAND_SIGNALINPUT:
            SignalInput = LimitValue(ValueReceived, 1, 5);
            Output_ShowValue( OCHAN_INPUT, SignalInput );
            Output_ShowEvent( EVENT_SHOW_INPUT, 1000 );  
            ESP_NOW_Send_Command( 0, 100 + COMMAND_SIGNALINPUT, 0, SignalInput);  
          break;

        case COMMAND_MAXVALUE:
            Process_SetParameters(COMMAND_MAXVALUE, 0, 0);
          break;

        case COMMAND_DISPLAYMODE:
            Output_SetDisplayMode( LimitValue(ValueReceived, 0, 2) );
            Output_Refresh(REFRESHTYPE_FULL);
          break;

        case COMMAND_FILTERBANK:
            currentValue = Process_SetParameters(COMMAND_FILTERBANK, 0, LimitValue(ValueReceived, 0, 2) );
            Output_SetValue( OCHAN_FILTERBANK, currentValue );
            Output_ShowEvent( EVENT_SHOW_FILTERBANK, 2000 ); 
          break;

        //default:
            //Process_SetParameters(CommandReceived, AudioOutputChannelReceived, ValueReceived);
            //Output_SetValue( OCHAN_AUDIO_PARAM, ValueReceived );
            //Output_ShowEvent( EVENT_SHOW_AUDIO_PARAM, 2000 );
    }
          
}




void loop() 
{

  int32_t NewMaxValueInputLeft =  Process_GetParameter(COMMAND_MAXVALUE, AUDIOINPUT_CHANNEL_LEFT);
  int32_t NewMaxValueInputRight = Process_GetParameter(COMMAND_MAXVALUE, AUDIOINPUT_CHANNEL_RIGHT);
  if ( ( NewMaxValueInputLeft != CurrentMaxValueInputLeft ) || ( NewMaxValueInputRight != CurrentMaxValueInputRight ) ) 
  {
    CurrentMaxValueInputLeft =  NewMaxValueInputLeft;
    CurrentMaxValueInputRight = NewMaxValueInputRight;
    Output_SetValue(  OCHAN_MAXVALUE_LEFT,  ( CurrentMaxValueInputLeft  * 1000 ) / 327680 ); 
    Output_ShowValue( OCHAN_MAXVALUE_RIGHT, ( CurrentMaxValueInputRight * 1000 ) / 327680 ); 
  }

  if (ESP_NOW_Received() == 1)
  {
      int32_t ValueReceived =   ESP_NOW_GetIntParam();
      uint8_t CommandReceived = ESP_NOW_GetCommand();
      uint8_t AddressReceived = ESP_NOW_GetAddress();
      uint8_t AudioOutputChannelReceived = 0;

      if ( ( AddressReceived == 0 ) || ( AddressReceived == DeviceAddress ) )
      {
        CommandAndDisplay(CommandReceived, AudioOutputChannelReceived, ValueReceived);       
      }
  }


  if (Serial_Received() == 1)
  {
    int32_t ValueReceived =   Serial_GetIntParam();
    uint8_t CommandReceived = Serial_GetCommand();
    uint8_t AudioOutputChannelReceived = Serial_GetByteParam();
    CommandAndDisplay(CommandReceived, AudioOutputChannelReceived, ValueReceived);       
  } 


  Input_Loop();

  int8_t iChannel = Input_GetChannelOfChangedValue();
  if (iChannel > -1)
  {
    int32_t iValue = Input_GetValue(iChannel);
    Serial.println( String(iChannel) + ":" + String(iValue) );

    if (iChannel == ICHAN_STEP_VOLUME1)
    {
      int32_t currentVolume = Process_GetParameter(COMMAND_VOLUME, 0);
      currentVolume = LimitValue(currentVolume + iValue, 0, 150);
      CommandAndDisplay(COMMAND_VOLUME, 0, currentVolume);       
    }

    if (iChannel == ICHAN_STEP_VOLUME2)
    {
      int32_t currentBassBalance = Process_GetParameter(COMMAND_BASSBALANCE, 0);
      currentBassBalance = LimitValue(currentBassBalance + iValue, 50, 170);
      CommandAndDisplay(COMMAND_BASSBALANCE, 0, currentBassBalance);    
    }

    if (iChannel == ICHAN_MODE)
    {
      CommandAndDisplay(COMMAND_SIGNALINPUT, 0, iValue);   
    }

    if (iChannel == ICHAN_SET)
    {
      if ( iValue == 1 ) CommandAndDisplay(COMMAND_MAXVALUE, 0, 0); 
      if ( iValue == 2 ) CommandAndDisplay(COMMAND_DISPLAYMODE, 0, 0); 
      if ( iValue == 3 ) CommandAndDisplay(COMMAND_DISPLAYMODE, 0, 1); 
      if ( iValue == 4 ) CommandAndDisplay(COMMAND_DISPLAYMODE, 0, 2); 
    }

  }

  Output_EventLoop();

}


