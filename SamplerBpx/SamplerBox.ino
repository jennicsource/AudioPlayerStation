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
// #include "Button.h"          // Button handling library
#include "Stream_I2S.h"      // I2S audio output functions
#include "Stream_RS485.h"
#include "Stream_TestSignals.h"


// === INITIALIZATION (Runs Once at Startup) ===
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("Sampler starting");

  RS485_Start(UART_RX, UART_TX);
  delay(200);


  // --- I2S (Audio Output) Setup ---
  AudioMode = AUDIO_MODE_SAMPLER;
  samplebuffer_count = 2;
  samplebuffer_length = 8;
  samplerate = 32000;        // audio sample rate
  audiochannelcount = 2;     // Stereo
  externalclock = 0;
  I2S_Start();
  delay(200);

  Serial.println("Init done!");

  TestSignal_SetType(TESTSIGNAL_TRIANGLE_STEREO);

  pinMode(PIN_SWITCH, INPUT_PULLUP);

  // --- Button Setup ---
  //Button_Init();  

}


uint8_t buttonp = 0;             // Button event value (0=no event, 1=short press, 2=long press)


// === GLOBAL BUFFERS FOR AUDIO DATA ===
uint8_t byteBuffer[128];        // Buffer for audio sample data
int16_t mBuffer[32];

void loop() 
{
      if ( digitalRead(PIN_SWITCH) == 1 ) 
      {
        I2S_ReadBytesInBuffer(byteBuffer, 32);
        RS485_WriteBytesFromBuffer(byteBuffer, 32);  
        delayMicroseconds(10); 
      }
      else
      {
        I2S_ReadBytesInBuffer(byteBuffer, 32);
        TestSignal_ReadSamplesInBuffer(mBuffer, 16);
        RS485_WriteSamplesFromBuffer(mBuffer, 16);
        delayMicroseconds(10);
      }      
}




