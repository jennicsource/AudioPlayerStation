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

#include "Stream_RS485.h"    // library for audio stream via RS485

#include "CoreZero.h"         // the functions which run on CoreZero are integrated here


uint8_t DeviceAddress = 1;    // Device Address, will be configured by DIP switch
uint8_t Tuning_Live = 1;      // Tuning in action or not


void setup() 
{
  
  Serial.begin(115200);       // start serial interface
  delay(200);

  Serial.println("Audioboard starting");

  RS485_Start(UART_RX, UART_TX);      // start the hardware serial interface for fast audio transmission (3 Mbit/s) 

  Radio_Init_Dual(500);          // initialize both nRF24 modules, with a timeout of 500 ms for switching the active module
  delay(100);
 
  Radio_Start_Dual(pCE, pCS, pCE2, pCS2);            // start both nRF24 modules

  Wire.begin(I2C_SDA, I2C_SCL, 400000);   // start I2C bus, you need that command once in the code

  Config_Init();  // init the module to control the non-volatile memory

  CurrentCorrection =  Config_GetValue(CCHAN_CORRECTION);   // get the correction for the clock in ppb from the non-volatile memory
  
  if (CurrentCorrection < -200000 || CurrentCorrection > 200000)   // avoid crazy random values
  {
	  CurrentCorrection = 0;
  }

 
  SignalInput = SIGNAL_AIR;        // Wireless transmission
  //SignalInput = SIGNAL_DIGITAL;  // RS485 can be selected here
  

  // --- I2S (Audio Output) Setup ---
  AudioMode = AUDIO_MODE_PLAYER;
  samplebuffer_count = 4;
  samplebuffer_length = 8;
  samplerate = 32000;        // audio sample rate
  
  audiochannelcount = 2;     // Stereo
  expandchannels = 0;        
  externalclock = 1;         // we use the external clock, so ESP32 in slave mode
  volumecontrol = 0;         // we use no volume control in this firmware version
  I2S_Start();
  delay(200);

  I2S_Clock_Init(SAMPLERATE_INDEX_32KHZ_16BIT, CurrentCorrection);   // start the clock with the current correction

  Sync_Init(8);    // Initialize Live-Tuning with a GateTimeFactor of 8 = 1250000 packets
  
  StartUARTTask();    // start the task for the UART/RS485 receiving of packets in the Core0 module
  StartAudioTask();   // start the task for the audio (receiving packets, processing, output to the DAC) in the Core0 module
 
}






void loop() 
{
  // this will be filled by remote control options in next firmware version
}


