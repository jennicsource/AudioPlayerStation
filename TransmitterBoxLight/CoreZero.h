



// these buffers for samples and bytes are needed to store data
int16_t mBuffer[PACKET_LENGTH_SAMPLES];
int16_t mBuffer2[PACKET_LENGTH_SAMPLES];
int16_t zeroBuffer[PACKET_LENGTH_SAMPLES] = {0};
int32_t exmBuffer[32];
int32_t zeroexmBuffer[32] = {0};

uint8_t byteBuffer[128]; 
int16_t musicBuffer[32];


#include <esp_task_wdt.h>             // this ESP32 lib is needed to reset the watchdog regularly, otherwise the controller will be reset after short amount of time
TaskHandle_t AudioTaskHandle = NULL;  // needed for task programming
TaskHandle_t UARTTaskHandle = NULL;

uint8_t SetupDone = 0;                // the tasks contain endless loops. if you want to do something just at the start, you must set this flag afterwards, and the setup wont be processed again
int32_t lastWatchdogReset = 0;        // needed to regularly reset the watchdog




uint8_t LastPacketNumber;
uint8_t PacketNumber;
int32_t LastFullPacketNumber;

int32_t OverflowPacketNumber;

int32_t LastCValue = 0;
uint8_t ValidPacket;


uint8_t PacketReceivingModule;


void DMXTask(void *parameter)         // one of the tasks running on Core 0
{
  for (;;) 
  { // Infinite loop
    
    //if (dmxSend.readyToTransmit()) 
    //{  // readyToTransmit uses the available space in the 
                              // Tx buffer + Tx FIFO as a reference set during configure
    //if (1) { // uncomment this line and comment the previous one to confirm it works as intended
    
    /*
      dmxSend.transmit();  // transmit blocks for the duration of the previous transmission
                          // plus the duration of the break. 
                          
      
      dmxSend.write(light, 27);
      dmxSend.write(light, 28);
      dmxSend.write(light, 29);
    */
      if (light == 40) digitalWrite(PIN_LED, HIGH);
      if (light == 0) digitalWrite(PIN_LED, LOW);
    

    //}  
  }
}



uint8_t AuxParam = 0;
//uint8_t PacketNumber = 0;

void AudioTask(void *parameter)    // the task for Audio processing
{
  for (;;) 
  { // Infinite loop
       
      if (SetupDone == 0)   // this code is only processed at startup
      {
        SetupDone = 1;
        
        // Initialize the watchdog with a 10-s timeout and panic mode enabled
        esp_task_wdt_init(10000, true);
        esp_task_wdt_add(NULL);
      } 

      
      if ( Radio1_DataReceived() == 1 )
      {

        PacketNumber = (uint8_t)Radio_ReadSamplesInBufferGetPacketNumber(1, mBuffer, 16);
        
        Process_Process(mBuffer, exmBuffer, 16);
        AuxParam = Process_GetAuxParameter(); 

        if (SignalFlag == 0)
        {
          if (ColorMode == 0)
          {
            if (AuxParam == 255)
            {
              digitalWrite(PIN_AUX_AUDIO2, LOW);
              digitalWrite(PIN_AUX_AUDIO, LOW);
            }
            else
            {
              digitalWrite(PIN_AUX_AUDIO2, LOW);
              digitalWrite(PIN_AUX_AUDIO, LOW);
            }
          }

          if (ColorMode == 1)
          {
            if (AuxParam == 255)
            {
              digitalWrite(PIN_AUX_AUDIO2, LOW);
              digitalWrite(PIN_AUX_AUDIO, HIGH);
            }
            else
            {
              digitalWrite(PIN_AUX_AUDIO2, LOW);
              digitalWrite(PIN_AUX_AUDIO, LOW);
            }
          }

          if (ColorMode == 2)
          {
            if (AuxParam == 255)
            {
              digitalWrite(PIN_AUX_AUDIO2, HIGH);
              digitalWrite(PIN_AUX_AUDIO, LOW);
            }
            else
            {
              digitalWrite(PIN_AUX_AUDIO2, LOW);
              digitalWrite(PIN_AUX_AUDIO, LOW);
            }
          }

          if (ColorMode == 3)
          {
            if (AuxParam == 255)
            {
              digitalWrite(PIN_AUX_AUDIO2, HIGH);
              digitalWrite(PIN_AUX_AUDIO, HIGH);
            }
            else
            {
              digitalWrite(PIN_AUX_AUDIO2, LOW);
              digitalWrite(PIN_AUX_AUDIO, LOW);
            }
          }

          if (ColorMode == 4)
          {
            if (AuxParam == 255)
            {
              digitalWrite(PIN_AUX_AUDIO2, HIGH);
              digitalWrite(PIN_AUX_AUDIO, LOW);
            }
            else
            {
              digitalWrite(PIN_AUX_AUDIO2, LOW);
              digitalWrite(PIN_AUX_AUDIO, HIGH);
            }
          }

        }

      }
      



      if (millis() - lastWatchdogReset >= 5000)   // we must reset the watchdog timer regularly in a time interval under 10 s
      {  
        esp_task_wdt_reset();  //  see https://f1atb.fr/esp32-watchdog-example/
        
        lastWatchdogReset = millis();
        //Serial.println("Reset");
      }

    

  }

}





void StartAudioTask() 
{
  xTaskCreatePinnedToCore(
    AudioTask,         // Task function
    "AudioTask",       // Task name
    100000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &AudioTaskHandle,  // Task handle
    0                // Core 0
  );
}