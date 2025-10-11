
// output channels to visualize something, but most are not used here
#define OCHAN_ERROR          0
#define OCHAN_DEVICENUMBER   1
#define OCHAN_STATUS         2
#define OCHAN_ACTIVE_MODULE  3
#define OCHAN_VOLUME         4
#define OCHAN_RADIOCHANNEL1   5
#define OCHAN_RADIOCHANNEL2   6

#define EVENT_POWER_ON               1
#define EVENT_RESET                  2

#define REFRESHTYPE_LEDONLY          1
#define REFRESHTYPE_FULL             2


#include "SevenSegmentDisplay.h"

// we need this for timing if an event occurred which 
// should be visualized with priortity for a certain amount of time
int32_t Output_EventTime = 0;
int32_t Output_EventTimeOut = 0;
int32_t Output_Event = 0;


int Output_Value[8];  // we have 8 values which can be visualized


// this is called once at the beginning, to initiate all visualizing elements
void Output_Init()
{
  // in that case, we initiate a LED which is active high
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW); 

  // and a 4 digit 7 segment display
  //SevenSegmentDisplay_Init(PIN_LEDDISPLAY_CLOCK, PIN_LEDDISPLAY_DATA);
};



// set the value of a certain channel to be visualized
void Output_SetValue(int Channel, int Value)
{
  Output_Value[Channel] = Value;
}


// call when an event occurred which should be visualized with priority for a certain amount of time
void Output_ShowEvent(int Event, int EventTimeOut)
{
  Output_EventTime = millis();
  Output_Event = Event;
  Output_EventTimeOut = EventTimeOut;
  
  // an Event occurred so visualize this with a LED on
  digitalWrite(PIN_LED, HIGH); 
}


// call this regularly to check if the time has ended for the event to be visualized
void Output_EventLoop()
{
  if (Output_Event != 0)
  {
    if ((millis() - Output_EventTime) > Output_EventTimeOut)
    {
      // end of visualization of the Event
      digitalWrite(PIN_LED, LOW); 
        
      Output_Event = 0;
    }
  }
}


// call this regularly to visulize all the values and check if the time has ended for the event to be visualized
void Output_Refresh(uint8_t RefreshType)
{
  if (Output_Event != 0)
  {
    if ((millis() - Output_EventTime) > Output_EventTimeOut)
    {
      // end of visualization of the Event
      digitalWrite(PIN_LED, LOW); 
        
      Output_Event = 0;
    }
  }
  else
  {
    // no prioritized event to show, so we can show in a normal mode all the values
    if (Output_Value[OCHAN_ACTIVE_MODULE] == 2)
    {
      // Value on the Active Module Channel is 2
      digitalWrite(PIN_LED, HIGH); 
    }
    else
    {
      // Value on the Active Module Channel is 1
      digitalWrite(PIN_LED, LOW); 
    }   
    
    if (RefreshType == REFRESHTYPE_FULL)
    {
      if (Output_Value[OCHAN_ERROR] == 0)
      {
        //SevenSegmentDisplay_ShowNumber(1000 + 10 * (Output_Value[OCHAN_RADIOCHANNEL1] * 2 + 1 ) + (Output_Value[OCHAN_RADIOCHANNEL2] * 2 ) );
      }
      else
      {
        //SevenSegmentDisplay_ShowNumber(9999);
      }
    }
  
  } 

}

// set a value to be visualized
// and visualize it immediately
void Output_ShowValue(int Channel, int Value)
{
  uint8_t RefreshType = REFRESHTYPE_LEDONLY;
  
  Output_Value[Channel] = Value;

  if ( (Channel == OCHAN_RADIOCHANNEL1) || (Channel == OCHAN_RADIOCHANNEL2) || (Channel == OCHAN_ERROR) ) RefreshType = REFRESHTYPE_FULL;
  Output_Refresh(RefreshType);
}


