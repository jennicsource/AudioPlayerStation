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

#include "BoardPin.h"        // Board-specific pin definitions

#include "Stream_I2S.h"           // I2S audio functions
#include "Stream_RS485.h"         // library for audio stream over RS485



// === INITIALIZATION (Runs Once at Startup) ===
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("Sampler starting");

  RS485_Start(UART_RX, UART_TX);      // start UART/RS485 with 3 Mbit/s 
  delay(200);


  // --- I2S (Audio Output) Setup ---
  AudioMode = AUDIO_MODE_SAMPLER;      // I2S driver is used for sampling 
  samplebuffer_count = 2;
  samplebuffer_length = 8;
  samplerate = 32000;        // audio sample rate
  audiochannelcount = 2;     // Stereo
  externalclock = 0;         // no external clock, so use ESP32 as master
  I2S_Start();
  delay(200);



}



// === GLOBAL BUFFERS FOR AUDIO DATA ===
uint8_t byteBuffer[128];        // Buffer for audio sample data


void loop() 
{
      
        I2S_ReadBytesInBuffer(byteBuffer, 32);        // sample date
        RS485_WriteBytesFromBuffer(byteBuffer, 32);   // and put to the RS485 bus  
        delayMicroseconds(10);                        // recommended for stability
     
     
}




