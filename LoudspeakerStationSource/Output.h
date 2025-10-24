
// output channels - all values to be visualized by a LED bar or 7 Segment display 
#define OCHAN_ERROR          0
#define OCHAN_DEVICENUMBER   1
#define OCHAN_WIFISTATUS     2
#define OCHAN_LEVEL_VOLUME   3
#define OCHAN_LEVEL_VOLUME1  3
#define OCHAN_LEVEL_VOLUME2  4
#define OCHAN_RADIO1_INDEX   5
#define OCHAN_RADIO2_INDEX   6
#define OCHAN_INPUT          7
#define OCHAN_AUDIO_PARAM    8


// different operation modes to be visualized by 7 Segment display
#define WIFI_CONNECTED      2
#define WIFI_NOTCONNECTED   1

#define ERROR_ACTUATOR      1
#define ERROR_SENSOR        2
#define ERROR_CONFIG        3


#define EVENT_MESSAGE_RECEIVED       1
#define EVENT_SHOW_RADIO1            2
#define EVENT_SHOW_RADIO2            3
#define EVENT_SHOW_INPUT             4
#define EVENT_SHOW_AUDIO_PARAM       5

#define REFRESHTYPE_LEDONLY          1
#define REFRESHTYPE_FULL             2


#include "SevenSegmentDisplay.h"


long Output_EventTime = 0;
int Output_Event = 0;
int32_t Output_EventTimeOut = 0;

int Output_Value[12];

int NumberToBeShown = 0;


void Output_Init()
{
  //SevenSegmentDisplay_Init(PIN_LEDBAR_CLOCK, PIN_LEDBAR_DATA);
};



void Output_SetValue(int Channel, int Value)
{
  Output_Value[Channel] = Value;
}


int Output_GetValue(int Channel)
{
  return Output_Value[Channel];
}


void Display_ShowNumber(uint16_t Number)
{
  oled.clearDisplay();
  oled.setTextSize(4); // 
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(10, 0);
  oled.println(String(Number));
  oled.display();      // Show initial text
}



void Output_ShowEvent(int intEvent, int EventTimeOut)
{
  Output_EventTime = millis();
  Output_Event = intEvent;
  Output_EventTimeOut = EventTimeOut;
  
  if (intEvent == EVENT_MESSAGE_RECEIVED) Display_ShowNumber(8888); 
  if (intEvent == EVENT_SHOW_AUDIO_PARAM)  Display_ShowNumber(Output_Value[OCHAN_AUDIO_PARAM]); 
  if (intEvent == EVENT_SHOW_RADIO1)  Display_ShowNumber(2 * Output_Value[ OCHAN_RADIO1_INDEX ] + 1); 
  if (intEvent == EVENT_SHOW_RADIO2)  Display_ShowNumber(2 * Output_Value[ OCHAN_RADIO1_INDEX ] ); 
  if (intEvent == EVENT_SHOW_INPUT)  Display_ShowNumber(1111 * Output_Value[OCHAN_INPUT]); 
 
}



// call this regularly to check if the time has ended for the event to be visualized
void Output_EventLoop()
{
  if (Output_Event != 0)
  {
    if ((millis() - Output_EventTime) > Output_EventTimeOut)
    {
        Output_Event = 0;
        Display_ShowNumber(NumberToBeShown); 
    }
  }
}



void Output_Refresh(uint8_t RefreshType)
{
  if (Output_Event != 0)
  {
    if ((millis() - Output_EventTime) > 700)
    {
        Output_Event = 0;
        Display_ShowNumber(NumberToBeShown); 
    }
  }
  else
  {
    if (Output_Value[OCHAN_ERROR] == 0)
    {
      if (Output_Value[OCHAN_WIFISTATUS] == WIFI_CONNECTED)    // Connected
      {
        NumberToBeShown = Output_Value[OCHAN_DEVICENUMBER] * 1000 + Output_Value[OCHAN_LEVEL_VOLUME1];
        Display_ShowNumber(NumberToBeShown); 
      }
      else                 // not connected
      {
        NumberToBeShown = Output_Value[OCHAN_LEVEL_VOLUME1];
        Display_ShowNumber(NumberToBeShown); 
      }
      
    }
    else
    {
      NumberToBeShown = 9900 + Output_Value[OCHAN_ERROR];
      Display_ShowNumber(NumberToBeShown); 
    }
       
  }

}



void Output_ShowValue(int Channel, int Value)
{
  Output_Value[Channel] = Value;
  Output_Refresh(REFRESHTYPE_FULL);
}

String Output_EjectVirtualDisplay()
{
  return "-" ;
}
