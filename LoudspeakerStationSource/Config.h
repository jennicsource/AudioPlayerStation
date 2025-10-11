
#include "DIPSwitch.h"


// the 3 channels for the configuration, 3 constant values are set by DIP switches at the start of the device
#define CCHAN_DOUBLESTATION         0
#define CCHAN_DONOTCONNECT_WIFI     1
#define CCHAN_DEVICENUMBER          2
#define CCHAN_DONOTSEND_WIFI        3



int Config_Value[8];


int Config_GetValue(int Channel)
{
  return Config_Value[Channel];
}



int Config_Init()
{
  int Success = 0;
 
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);

  if (DIPSwitch_Init() != 1) 
  {
    Serial.println("Device failed");
    Success = -1;

  }
  else
  {
    delay(10);
    
    int StationNumber = 0;
    int SwitchValue = 1;
    
    for (int i = 0; i < 3; i++) 
    {
      if (DIPSwitch_GetSwitch(i) == 1)
      {
        StationNumber +=  SwitchValue;
      }
      SwitchValue = SwitchValue * 2;
    }  

    int StationDoNotSend =  0;
    if (DIPSwitch_GetSwitch(3) == 1)
    {
      StationDoNotSend =  1;
    }

    int StationDouble =  0;
    if (DIPSwitch_GetSwitch(4) == 1)
    {
      StationDouble =  1;
    }

    int StationConfig =  0;
    if (DIPSwitch_GetSwitch(5) == 1)
    {
      StationConfig =  1;
    }
    
    //Serial.println("StationNumber = " + String(StationNumber));

    Config_Value[CCHAN_DEVICENUMBER] = StationNumber;
    Config_Value[CCHAN_DOUBLESTATION] = StationDouble;
    Config_Value[CCHAN_DONOTCONNECT_WIFI] = StationConfig;
    Config_Value[CCHAN_DONOTSEND_WIFI] = StationDoNotSend;
    
  }

  return Success;

}


