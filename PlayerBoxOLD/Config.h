
#include "DIPSwitch.h"

#include <Preferences.h>   // this library is used to get access to Non-volatile storage (NVS) of ESP32
// see https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/examples/StartCounter/StartCounter.ino
// https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/src/Preferences.h

// the channels for the configuration values
#define CCHAN_CORRECTION         0
#define CCHAN_DEVICE_ADDRESS     1
#define CCHAN_DEVICE_SUBSAT      2


Preferences preferences;  // ESP32 library for non-volatile storage

int32_t Config_Value[8];  // up to 8 configuration values


int32_t Config_GetValue(uint8_t Channel)
{
  return Config_Value[Channel];        // get one configuration value
}



void Config_SetValue(uint8_t Channel, int32_t Value)
{
  Config_Value[Channel] = Value;      // set one configuration value
}


void Config_StoreValues()
{
  preferences.putInt("Correction", Config_Value[CCHAN_CORRECTION]);  // store the correction value in the non-volatile memory
}



uint8_t Config_Init()   // called at startup
{
  uint8_t Success = 1;
 
  for (int Channel = 0; Channel < 8; Channel++)
  {
    Config_Value[Channel] = 0;
  }
  
  preferences.begin("AVS", false);
  int32_t CorrectionFactor = preferences.getInt("Correction", 0);  // get the configuration value out of the non-volatile memory

  Config_Value[CCHAN_CORRECTION] = CorrectionFactor;  // store the value so it can be requested by the main program
    
  if (DIPSwitch_Init() != 1)       // start the DIP switch library
  {
    Serial.println("DIP Device failed");
    Success = -1;
  }
  else
  {
    delay(10);
    
    // read out 6 DIP switches
    int StationNumber = 0;
    int SwitchValue = 1;
    
    // these 3 are for the device address
    for (int i = 0; i < 3; i++) 
    {
      if (DIPSwitch_GetSwitch(i) == 1)
      {
        StationNumber +=  SwitchValue;
      }
      SwitchValue = SwitchValue * 2;
    }  

    // this one says if the board sends back values or not via ESP-NOW (not active at the moment)
    int StationDoNotSend =  0;
    if (DIPSwitch_GetSwitch(3) == 1)
    {
      StationDoNotSend =  1;
    }

    // this one says if the board connects via ESP-NOW (not active at the moment)
    int StationDoNotConnect =  0;
    if (DIPSwitch_GetSwitch(4) == 1)
    {
      StationDoNotConnect =  1;
    }

    // this one says if the board should play in Sub/Sat configuration
    int StationSubSat =  0;
    if (DIPSwitch_GetSwitch(5) == 1)
    {
      StationSubSat =  1;
    }
    
    Serial.println("StationNumber = " + String(StationNumber));

    Config_Value[CCHAN_DEVICE_ADDRESS] = StationNumber;  // store the value so it can be requested by the main program
    Config_Value[CCHAN_DEVICE_SUBSAT]  = StationSubSat;  // store the value so it can be requested by the main program
  }

  return Success;

}


