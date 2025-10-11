


uint8_t buttonState = 0;
uint32_t buttonOn = 0;
uint32_t buttonTime = 0;

// 4 Buttons are mapped on 2 Input Channels

#define INPUT_VALUES_MAX   2



int Input_Value[INPUT_VALUES_MAX];
bool Input_Value_Changed[INPUT_VALUES_MAX];



void Input_Init()
{
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  for (int m = 0; m < INPUT_VALUES_MAX ; m++)
  {
    Input_Value_Changed[m] =    false;
  }
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
      Input_Value_Changed[m] = false;
      Result = m;
      break;
    }
    
  }
  return Result;
}



void Input_Loop()
{
  
  uint8_t button = !(digitalRead(PIN_BUTTON)); 

  if ((buttonState == 0) && (button == 1))
  {
    buttonState = button;
    buttonOn = millis(); 
  } 

  if ((buttonState == 1) && (button == 0))
  {
    buttonState = button;
    buttonTime = millis() - buttonOn;

    if (buttonTime > 1000)
    {
      Serial.println("Button Long");
      
      
      Input_Value_Changed[0] = true;
    }

    if ((buttonTime > 10) && (buttonTime < 1000))
    {
      Serial.println("Button Short");

      
      Input_Value_Changed[1] = true;
    }

  } 
    
}

