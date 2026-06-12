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

uint8_t light = 0;

int32_t samplerate = 32000; 

#include "BoardPin.h"        // Board-specific pin definitions

#include "Radio_nRF24.h" 
//#include "Stream_RS485.h"   

#include "Stream_Process.h"  // library for Audio processing (Volume, Filters...)

#include "ESP_NOW_Control.h"  // library for remote control via ESP-NOW
#include "Serial_Control.h"   // library for remote control via Serial interface

uint8_t DeviceAddress = 1;
uint8_t ColorMode = 1;

#include "CoreZero.h"         // the functions which run on CoreZero are integrated here
 

void setup() 
{
  Serial.begin(115200);
  delay(100);
  Serial.println("Gateway starting");

  samplerate = 32000;        // audio sample rate

  ESP_NOW_Connect(ESP_NOW_RECEIVER, 1);   // start ESP-NOW connection with this board as receiver

  //RS485_Start(UART_RX, UART_TX);  //  start UART with 3 MBit/s

  pinMode(PIN_AUX_AUDIO, OUTPUT);  // switch 
  pinMode(PIN_AUX_AUDIO2, OUTPUT);  // switch 

  //digitalWrite(PIN_SWITCH, HIGH);

  Radio_Init(1);                      // 1 for default channel 90, 2 for default channel 125. this must be different when you use 2 radio modules sending!

  Radio_Start(1, RADIO_MODE_RECEIVER, pCE, pCS);   //  start radio module 1 as sender

  Serial.println("radio started");

  Process_SetParameters(COMMAND_LOWPASS,        AUDIOOUTPUT_CHANNEL_AUX, 90);

  digitalWrite(PIN_AUX_AUDIO2, HIGH);
  digitalWrite(PIN_AUX_AUDIO, HIGH);
  delay(400);
  digitalWrite(PIN_AUX_AUDIO2, LOW);
  digitalWrite(PIN_AUX_AUDIO, LOW);

  StartAudioTask();   // start the task for the audio (receiving packets, processing, output to the DAC) in the Core0 module
 
}



// a helper function to limit a value to an upper and lower limit
int32_t LimitValue(int32_t CurrentValue, int32_t LowerLimit, int32_t UpperLimit)
{
  int32_t ResultValue = CurrentValue;
  if (ResultValue < LowerLimit) ResultValue = LowerLimit;
  if (ResultValue > UpperLimit) ResultValue = UpperLimit;
  return ResultValue;
}

// a helper function to limit a value to an upper and lower limit, but when exceeding the upper limit, it goes to the lower limit, and vice versa
int32_t CycleValue(int32_t CurrentValue, int32_t LowerLimit, int32_t UpperLimit)
{
  int32_t ResultValue = CurrentValue;
  if (ResultValue < LowerLimit) ResultValue = UpperLimit;
  if (ResultValue > UpperLimit) ResultValue = LowerLimit;
  return ResultValue;
}


#define COMMAND_DISPLAYMODE  89   // Y
#define COMMAND_DISPLAYMODE2 90   // Z


// this is the function to process a received command
void CommandAndDisplay(uint8_t CommandReceived, uint8_t AudioOutputChannelReceived, int32_t ValueReceived)
{
    int32_t currentValue;
    
    switch(CommandReceived)
    {
        
        case COMMAND_DELAY:
            ValueReceived = LimitValue(ValueReceived, 0, 2000);    // Limit delay parameter to 0..2000
            currentValue = Process_SetParameters(CommandReceived, AudioOutputChannelReceived, ValueReceived);  // set new delay
 
            ESP_NOW_Send_Command( DeviceAddress, 100 + COMMAND_DELAY, AudioOutputChannelReceived, currentValue );  
          break;

        case COMMAND_STEREOWIDTH:
            ValueReceived = LimitValue(ValueReceived, -100, 200);
            currentValue = Process_SetParameters(CommandReceived, AudioOutputChannelReceived, ValueReceived);
         
            ESP_NOW_Send_Command( DeviceAddress, 100 + COMMAND_STEREOWIDTH, AudioOutputChannelReceived, currentValue );
          break;

        
        case COMMAND_LOWPASS:
          
            AudioOutputChannelReceived = AUDIOOUTPUT_CHANNEL_SUB;   // the lowpass will just used for the sub output channel
            ValueReceived = LimitValue(ValueReceived, 20, 200);     // and it must stay between 20 and 200 Hz
           
            currentValue = Process_SetParameters(CommandReceived, AudioOutputChannelReceived, ValueReceived);

            delay(DeviceAddress * 50 + 70);  // this command is working on multiple player boxes, so all player boxes will send an answer
            ESP_NOW_Send_Command( 0, 100 + COMMAND_LOWPASS, AudioOutputChannelReceived, currentValue );            
          break;

        case COMMAND_RATIO:
            ValueReceived = LimitValue(ValueReceived, 10, 100);     // 
           
            currentValue = Process_SetParameters(CommandReceived, 0, ValueReceived);

            delay(DeviceAddress * 50 + 70);  // this command is working on multiple player boxes, so all player boxes will send an answer
            ESP_NOW_Send_Command( DeviceAddress, 100 + COMMAND_RATIO, 0, currentValue );            
          break;

        case COMMAND_DISPLAYMODE:
            ColorMode = LimitValue(ValueReceived, 0, 4);
            
            delay(DeviceAddress * 50 + 70);  // this command is working on multiple player boxes, so all player boxes will send an answer
            ESP_NOW_Send_Command(  DeviceAddress, 100 + COMMAND_DISPLAYMODE, 0, ColorMode );        
          break;

        case COMMAND_DISPLAYMODE2:
            
          break;

        case COMMAND_FILTERBANK:
            currentValue = Process_SetParameters(COMMAND_FILTERBANK, 0, LimitValue(ValueReceived, 0, 2) );
          break;

        //default:
            //Process_SetParameters(CommandReceived, AudioOutputChannelReceived, ValueReceived);
            //Output_SetValue( OCHAN_AUDIO_PARAM, ValueReceived );
            //Output_ShowEvent( EVENT_SHOW_AUDIO_PARAM, 2000 );
    }
          
}



void loop() 
{
     
  if (ESP_NOW_Received() == 1)    // we have a new ESP NOW message
  {
      int32_t ValueReceived =   ESP_NOW_GetIntParam();  // retrieve the value, the command, the device address, and the audio channel 
      uint8_t CommandReceived = ESP_NOW_GetCommand();
      uint8_t AddressReceived = ESP_NOW_GetAddress();
      uint8_t AudioOutputChannelReceived = ESP_NOW_GetByteParam();

      if ( ( AddressReceived == 0 ) || ( AddressReceived == DeviceAddress ) )
      {
        CommandAndDisplay(CommandReceived, AudioOutputChannelReceived, ValueReceived);   // process the command    
      }
  }

  if (ESP_NOW_Eight_Received() == 1)   // we have a new ESP NOW message, which receives 8 commands at the same time
  { 
      for (uint8_t pp = 0; pp < 8; pp++)  // process these 8 commands
      {
        int32_t ValueReceived =   ESP_NOW_Eight_GetIntParam(pp);
        uint8_t CommandReceived = ESP_NOW_Eight_GetCommand(pp);
        uint8_t AddressReceived = ESP_NOW_Eight_GetAddress(pp);
        uint8_t AudioOutputChannelReceived = ESP_NOW_Eight_GetByteParam(pp);

        if ( ( AddressReceived == 0 ) || ( AddressReceived == DeviceAddress ) )
        {
          CommandAndDisplay(CommandReceived, AudioOutputChannelReceived, ValueReceived);   // process the single command
        }
      }
  }
  

  if (Serial_Received() == 1)  // we have a new serial message
  {
    int32_t ValueReceived =   Serial_GetIntParam();
    uint8_t CommandReceived = Serial_GetCommand();
    uint8_t AudioOutputChannelReceived = Serial_GetByteParam();
    CommandAndDisplay(CommandReceived, AudioOutputChannelReceived, ValueReceived);       // process the command
  } 
 

}