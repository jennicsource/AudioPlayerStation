
#define BUTTON_1_UP     1
#define BUTTON_1_DOWN   2
#define BUTTON_2_UP     3
#define BUTTON_2_DOWN   4


// the 2 channels for IR receiver input, 2 buttons are used to control the first potentiometer and 2 buttons the second one
#define ICHAN_STEP_VOLUME1    0
#define ICHAN_STEP_VOLUME2    1


#include "Elem_I_IRReceiver.h"

Elem_I_IRReceiver InputElement0 = Elem_I_IRReceiver();


// 4 Buttons are mapped on 2 Input Channels

#define INPUT_VALUES_MAX   2



int Input_Value[INPUT_VALUES_MAX];
bool Input_Value_Changed[INPUT_VALUES_MAX];
String Input_Path[INPUT_VALUES_MAX];


void Input_Init()
{
  int Success = InputElement0.Init();

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


int Input_GetChannelOfChangedValue()
{
  int Result = -1;
  for (int m = 0; m < INPUT_VALUES_MAX ; m++)
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
  int ButtonReceived = InputElement0.Loop();
  
  if (ButtonReceived > -1) 
  {
    
    if (ButtonReceived == BUTTON_1_UP)
    {
        //Serial.println("Vol_A_+");
        Input_Value[ICHAN_STEP_VOLUME1] = 100;
        Input_Value_Changed[ICHAN_STEP_VOLUME1] = true;
    }

    if (ButtonReceived == BUTTON_1_DOWN)
    {
        //Serial.println("Vol_A_-");
        Input_Value[ICHAN_STEP_VOLUME1] = -100;
        Input_Value_Changed[ICHAN_STEP_VOLUME1] = true;
    }

    if (ButtonReceived == BUTTON_2_UP)
    {
        //Serial.println("Vol_B_+");
        Input_Value[ICHAN_STEP_VOLUME2] = 100;
        Input_Value_Changed[ICHAN_STEP_VOLUME2] = true;
    }

    if (ButtonReceived == BUTTON_2_DOWN)
    {
        //Serial.println("Vol_B_-");
        Input_Value[ICHAN_STEP_VOLUME2] = -100;
        Input_Value_Changed[ICHAN_STEP_VOLUME2] = true;
    }

  }

}


// Injection
void PushVirtualButton(int button)
{
  if (button == BUTTON_1_UP)
  {
      Input_Value[ICHAN_STEP_VOLUME1] = 100;
      Input_Value_Changed[ICHAN_STEP_VOLUME1] = true;      
  }

  if (button == BUTTON_1_DOWN)
  {
      Input_Value[ICHAN_STEP_VOLUME1] = -100;
      Input_Value_Changed[ICHAN_STEP_VOLUME1] = true;      
  }

}


void Input_InjectButtonPress(String ButtonPressCommand)
{
    if (ButtonPressCommand.indexOf("button1") > 0)
    {
      PushVirtualButton(1);
    }

   
    if (ButtonPressCommand.indexOf("button2") > 0)
    {
      PushVirtualButton(2);
    }

}

