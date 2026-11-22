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
#include "Stream_TestSignals.h"   // library to fill the samples to be output with a triangle test signal


#include "Compressor.h"

static Compressor compressor;




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
  levelcontrol = 1;          // 32 bit sampling
  externalclock = 0;         // no external clock, so use ESP32 as master
  I2S_Start();
  delay(200);

  TestSignal_SetType(TESTSIGNAL_TRIANGLE_STEREO);     // set the type of the test signal. a different tone for left and right loudspeaker

  pinMode( PIN_SWITCH, INPUT_PULLUP );                  // a switch is used to generate test signal
  pinMode( PIN_LED, OUTPUT );                  // a switch is used to generate test signal

  //digitalWrite( PIN_LED, HIGH );

  // Example: 48 kHz, 25% threshold, factor 4, 5 ms attack, 100 ms release.

  if ( !compressor_init( &compressor, 32000.0, 0.17, 16.0, 0.0, 20.0 ) ) 
  {
      /* Handle invalid parameters. */
  }


}



uint16_t inputlevel = 64;  // default is 64


// === GLOBAL BUFFERS FOR AUDIO DATA ===
uint8_t byteBuffer[128];        // Buffer for audio sample data
uint8_t inbyteBuffer[128];        // Buffer for audio sample data
int16_t mBuffer[32];
int32_t fBuffer[32];

int16_t ADC_Values1[256]= {0};
int16_t Cursor1 = 0;
int16_t intShowCycle = 0;
uint16_t levelValueOld;

void loop() 
{
      if ( digitalRead(PIN_SWITCH) == 1 )       // switch not active
      {
        //I2S_ReadBytesInBuffer(byteBuffer, 32);        // sample date
        I2S_ReadBytesInBuffer(inbyteBuffer, 64);        // sample date

        memcpy(fBuffer, inbyteBuffer, 64);

        for (uint8_t i = 0; i < 16; i++)
        {
           fBuffer[i] = ( fBuffer[i] >> 8 ) * inputlevel ;
        }

        uint8_t compressing = compressor_process(&compressor, fBuffer, 8);  /* 8 stereo frames */

        if (compressing == 1) digitalWrite( PIN_LED, HIGH );
        if (compressing == 0) digitalWrite( PIN_LED, LOW );

        for (uint8_t i = 0; i < 16; i++)
        {
           mBuffer[i] = int16_t( fBuffer[i] >> 14 );
        } 

        memcpy(byteBuffer, mBuffer, 32);
        
        RS485_WriteBytesFromBuffer(byteBuffer, 32);   // and put to the RS485 bus  
        delayMicroseconds(10);                        // recommended for stability

        uint16_t potValue = analogRead(PIN_POT); // reads pot value. target is 32 to 256
       
        ADC_Values1[Cursor1] = potValue;
        Cursor1 = Cursor1 + 1;
        if (Cursor1 > 63) Cursor1 = 0;
        
        intShowCycle++;
        if (intShowCycle > 15)
        {
          intShowCycle = 0;
          int32_t ADC_Sum1 = 0;
          for (uint16_t i = 0; i < 64; i++)
          {
            ADC_Sum1 = ADC_Sum1 + ADC_Values1[i];
          }
          uint16_t levelValue = ( ADC_Sum1 >> 10 ) + 48;
          if ( abs ( levelValue - inputlevel ) > 5 )
          {
            inputlevel = levelValue;
            Serial.println(levelValue);                // Gibt den Wert im Monitor aus
          }
          
          
          
        }


        
      }
      else                                     // switch active
      {
        I2S_ReadBytesInBuffer(byteBuffer, 32);       // sample data, but do not use it (this is for the right timing, as we want create data with a defined sample rate)
        TestSignal_ReadSamplesInBuffer(mBuffer, 16); // generate test signals and put in mBuffer
        RS485_WriteSamplesFromBuffer(mBuffer, 16);   // put the test signal data to the RS485 bus
        delayMicroseconds(10);
      }      
}




