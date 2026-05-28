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

#include "CoreZero.h"         // the functions which run on CoreZero are integrated here
 

void setup() 
{
  Serial.begin(115200);
  delay(400);
  Serial.println("Gateway starting");

  samplerate = 32000;        // audio sample rate

  //RS485_Start(UART_RX, UART_TX);  //  start UART with 3 MBit/s

  pinMode(PIN_AUX_AUDIO, OUTPUT);  // switch 
  pinMode(PIN_AUX_AUDIO2, OUTPUT);  // switch 

  //digitalWrite(PIN_SWITCH, HIGH);

  Radio_Init(1);                      // 1 for default channel 90, 2 for default channel 125. this must be different when you use 2 radio modules sending!

  Radio_Start(1, RADIO_MODE_RECEIVER, pCE, pCS);   //  start radio module 1 as sender

  Serial.println("radio started");

  Process_SetParameters(COMMAND_LOWPASS,        AUDIOOUTPUT_CHANNEL_AUX, 90);

  StartAudioTask();   // start the task for the audio (receiving packets, processing, output to the DAC) in the Core0 module
 
}




void loop() 
{
      

}