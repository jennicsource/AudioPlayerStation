
uint8_t SignalInput = SIGNAL_AIR;


// these buffers for samples and bytes are needed to store data
int16_t mBuffer[PACKET_LENGTH_SAMPLES];
int16_t mBuffer2[PACKET_LENGTH_SAMPLES];
int32_t exmBuffer[32];

uint8_t byteBuffer[128]; 
int16_t musicBuffer[32];


#include <esp_task_wdt.h>             // this ESP32 lib is needed to reset the watchdog regularly, otherwise the controller will be reset after short amount of time
TaskHandle_t AudioTaskHandle = NULL;  // needed for task programming
TaskHandle_t UARTTaskHandle = NULL;

uint8_t SetupDone = 0;                // the tasks contain endless loops. if you want to do something just at the start, you must set this flag afterwards, and the setup wont be processed again
int32_t lastWatchdogReset = 0;        // needed to regularly reset the watchdog



void UARTTask(void *parameter)         // one of the tasks running on Core 0
{
  for (;;) 
  { // Infinite loop
    
    if (SignalInput == SIGNAL_DIGITAL)   // getting data via RS485
    {
      int16_t PacketNumberOrInvalid = RS485_ReadBytesInBufferGetPacketNumber(byteBuffer, 32);   // read a packet of data via RS485 in byteBuffer and get the number of the data packet
    
      if ( PacketNumberOrInvalid != -1 )      // we received a valid packet
      {
        Sync_WritePacketNumberTestPacketCounterOnOverflow( (uint8_t)PacketNumberOrInvalid );     // write the packetnumber (0..255) to the tuning function, which also checks the overflow
      }
    }
    else
    {
      delay(1);   // in case of the input is not configured to RS485/UART, you must do something else, otherwise the program crashes
    }
  }
}





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

      int32_t TuneResult = Sync_Tune();     // call the tune function, which will calculate a new correction factor after a certain amount of data packets arrived, and then tune the clock, when needed
      if (TuneResult != TUNE_RESULT_NO_CORRECTION) 
      {
        I2S_Clock_Adapt(CurrentCorrection);
        
        Serial.println("TUNING - in ppb: " + String(TuneResult) );   
        Config_SetValue(CCHAN_CORRECTION, CurrentCorrection);       // put the new correction factor to the Config module
        Config_StoreValues();                                       // and store the new value in the non-volatile memory
      };
       

      if (SignalInput == SIGNAL_AIR)        // input via wireless transmission
      {  
        uint8_t PacketReceivingModule = Radio_PacketCheckDual();    // check if packet was received  

        //Output_ShowValue(OCHAN_ACTIVE_MODULE, PacketReceivingModule);   // visualize the active radio module via an LED, not used in this firmware version
        
        if (PacketReceivingModule == 1)  // packet received by module 1
        {         
          uint8_t PacketNumber = Radio_ReadSamplesInBufferGetPacketNumber(1, mBuffer, PACKET_LENGTH_SAMPLES);  // read the data from radio module 1

          I2S_WriteSamplesFromBuffer(mBuffer, 16);     // output the data to the DAC

          Sync_WritePacketNumberTestPacketCounterOnOverflow( PacketNumber );   // write the packetnumber (0..255) to the tuning function, which also checks the overflow
        }

        if (PacketReceivingModule == 2) // // packet received by module 2
        {         
          uint8_t PacketNumber = Radio_ReadSamplesInBufferGetPacketNumber(2, mBuffer2, PACKET_LENGTH_SAMPLES); // read the data from radio module 2

          I2S_WriteSamplesFromBuffer(mBuffer2, 16);    // output the data to the DAC

          Sync_WritePacketNumberTestPacketCounterOnOverflow( PacketNumber );   // write the packetnumber (0..255) to the tuning function, which also checks the overflow
        }
      }


      if (SignalInput == SIGNAL_DIGITAL)     // input via wireless transmission
      { 
        memcpy(mBuffer, byteBuffer, 32);     // the data are put regularly in the byteBuffer by the UARTtask
        I2S_WriteSamplesFromBuffer(mBuffer, 16);   // put the data to the DAC
      }  


      if (millis() - lastWatchdogReset >= 5000)   // we must reset the watchdog timer regularly in a time interval under 10 s
      {  
        esp_task_wdt_reset();  //  see https://f1atb.fr/esp32-watchdog-example/
        
        lastWatchdogReset = millis();
        //Serial.println("Reset");
      }

      Sync_TestPinCounterOnOverflow();    // call this function regularly in a time interval under 22050 (the upper limit of the counter) 
                                          // * wordclock frequency (which is sample frequency) = 0.68 seconds, to detect overflows

  }

}


// the functions to start the tasks running on Core 0
void StartUARTTask() 
{
 xTaskCreatePinnedToCore(
    UARTTask,         // Task function
    "UARTTask",       // Task name
    10000,             // Stack size (bytes)
    NULL,              // Parameters
    1,                 // Priority
    &UARTTaskHandle,  // Task handle
    0                  // Core 0
  );
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