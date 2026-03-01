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

#include "Radio_nRF24.h" 
#include "Stream_RS485.h"      
 

void setup() {
  Serial.begin(115200);
  delay(400);
  Serial.println("Gateway starting");

  RS485_Start(UART_RX, UART_TX);

  pinMode(PIN_SWITCH, INPUT_PULLUP);

  Radio_Init(1);
  //Radio1_Start(RADIO_MODE_SENDER);

  Radio_Start(1, RADIO_MODE_SENDER, pCE, pCS);

}


uint8_t byteBuffer[128];
int16_t musicBuffer[32];



void loop() {
  
      int16_t PacketNumberOrInvalid = RS485_ReadBytesInBufferGetPacketNumber(byteBuffer, 32);

      if ( PacketNumberOrInvalid != -1 )      // we received a valid packet
      {
        
        memcpy(&musicBuffer, &byteBuffer, 32);

        for (uint8_t s = 0; s < 16; s++)
        {      
          musicBuffer[s] = musicBuffer[s] &  0b1111111111111110;

          if (s < 8)
          {
            uint8_t bitresult = ( PacketNumberOrInvalid & ( 1 << s ) ) >> s;
            musicBuffer[s] = musicBuffer[s] +  bitresult;
          }

        } 

        memcpy(&byteBuffer, &musicBuffer, 32);
        
        if ( digitalRead(PIN_SWITCH) == 1 ) Radio_WriteBytesFromBuffer(byteBuffer, 32);

        delayMicroseconds(40);
      }

}