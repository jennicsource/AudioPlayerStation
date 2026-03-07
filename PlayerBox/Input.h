
#include "IRReceiver.h"

// the 2 channels for IR receiver input
#define ICHAN_STEP_VOLUME1    0
#define ICHAN_STEP_VOLUME2    1
#define ICHAN_MODE            2
#define ICHAN_ROUTING         3
#define ICHAN_SET             4



// Buttons are mapped on 8 Input Channels

#define INPUT_VALUES_MAX   8



int Input_Value[INPUT_VALUES_MAX];
bool Input_Value_Changed[INPUT_VALUES_MAX];
String Input_Path[INPUT_VALUES_MAX];


void Input_Init()
{
 
  IRReceiver_Init(PIN_IR);
 
  for (int m = 0; m < INPUT_VALUES_MAX ; m++)
  {
    Input_Value_Changed[m] =    false;
  }
}


void Input_SetPath(int Channel, String Path)
{
  Input_Path[Channel] = Path;
}


int Input_GetValue(int Channel)
{
  Input_Value_Changed[Channel] = false;
  
  return Input_Value[Channel];
}


int8_t Input_GetChannelOfChangedValue()
{
  int8_t Result = -1;
  for (int8_t m = 0; m < INPUT_VALUES_MAX ; m++)
  {
    if (Input_Value_Changed[m] == true) 
    {
      Result = m;
      break;
    }
    
  }
  return Result;
}



void Input_Loop()
{
  unsigned int IRCode = IRReceiver_Loop();
  
  if (IRCode > 0) 
  {
    //Serial.println(IRCode);
    
    if (IRCode == 0xE0E0E01F)
    {
        //Serial.println("Vol_A_+");
        Input_Value[ICHAN_STEP_VOLUME1] = 10;
        Input_Value_Changed[ICHAN_STEP_VOLUME1] = true;
    }

     if (IRCode == 0xE0E0D02F)
    {
        //Serial.println("Vol_A_-");
        Input_Value[ICHAN_STEP_VOLUME1] = -10;
        Input_Value_Changed[ICHAN_STEP_VOLUME1] = true;
    }

    if (IRCode == 0xE0E048B7)
    {
        //Serial.println("Vol_B_+");
        Input_Value[ICHAN_STEP_VOLUME2] = 10;
        Input_Value_Changed[ICHAN_STEP_VOLUME2] = true;
    }

    if (IRCode == 0xE0E008F7)
    {
        //Serial.println("Vol_B_-");
        Input_Value[ICHAN_STEP_VOLUME2] = -10;
        Input_Value_Changed[ICHAN_STEP_VOLUME2] = true;
    }

    if (IRCode == 3772784863)   //   Key 1
    {
        Input_Value[ICHAN_MODE] = 1;
        Input_Value_Changed[ICHAN_MODE] = true;
    }

    if (IRCode == 3772817503)   //   Key 2
    {
        Input_Value[ICHAN_MODE] = 2;
        Input_Value_Changed[ICHAN_MODE] = true;
    }

    if (IRCode == 3772801183)   //   Key 3
    {
        Input_Value[ICHAN_MODE] = 3;
        Input_Value_Changed[ICHAN_MODE] = true;
    }

    if (IRCode == 3772780783)   //   Key 4
    {
        Input_Value[ICHAN_MODE] = 4;
        Input_Value_Changed[ICHAN_MODE] = true;
    }

    if (IRCode == 3772813423)   //   Key 5
    {
        Input_Value[ICHAN_MODE] = 5;
        Input_Value_Changed[ICHAN_MODE] = true;
    }

    if (IRCode == 3772811383)   //   Key 0
    {
        Input_Value[ICHAN_SET] = 1;
        Input_Value_Changed[ICHAN_SET] = true;
    }

    if (IRCode == 3772788943)   //   Key 7
    {
        Input_Value[ICHAN_SET] = 2;
        Input_Value_Changed[ICHAN_SET] = true;
    }

    if (IRCode == 3772821583)   //   Key 8
    {
        Input_Value[ICHAN_SET] = 3;
        Input_Value_Changed[ICHAN_SET] = true;
    }

    if (IRCode == 3772805263)   //   Key 9
    {
        Input_Value[ICHAN_SET] = 4;
        Input_Value_Changed[ICHAN_SET] = true;
    }

    if (IRCode == 3772797103)   //   Key 6
    {
        Input_Value[ICHAN_SET] = 5;
        Input_Value_Changed[ICHAN_SET] = true;
    }



  }

}




