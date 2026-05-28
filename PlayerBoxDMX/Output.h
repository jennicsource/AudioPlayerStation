
// output channels - all values to be visualized by a LED bar or an OLED display 
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
#define OCHAN_ACTIVE_MODULE  9
#define OCHAN_BASSBALANCE    10
#define OCHAN_LOWPASS        11
#define OCHAN_HIGHPASS       12
#define OCHAN_MAXVALUE_LEFT  13
#define OCHAN_MAXVALUE_RIGHT 14
#define OCHAN_PARAMETRIC     15
#define OCHAN_FILTERBANK     16
#define OCHAN_TUNING         17
#define OCHAN_DELAY          18
#define OCHAN_DATAFLOW       19
#define OCHAN_COMPRESSOR     20
#define OCHAN_STEREOWIDTH    21
#define OCHAN_ROUTING        22


#define ERROR_ACTUATOR      1
#define ERROR_SENSOR        2
#define ERROR_CONFIG        3


#define EVENT_MESSAGE_RECEIVED       1
#define EVENT_SHOW_RADIO1            2
#define EVENT_SHOW_RADIO2            3
#define EVENT_SHOW_INPUT             4
#define EVENT_SHOW_AUDIO_PARAM       5
#define EVENT_SHOW_BASSBALANCE       6
#define EVENT_SHOW_LOWPASS           7
#define EVENT_SHOW_HIGHPASS          8
#define EVENT_SHOW_PARAMETRIC        9
#define EVENT_SHOW_FILTERBANK        10
#define EVENT_SHOW_DELAY             11
#define EVENT_SHOW_COMPRESSOR        12
#define EVENT_SHOW_STEREOWIDTH       13
#define EVENT_SHOW_ROUTING           14

#define REFRESHTYPE_LEDONLY          1
#define REFRESHTYPE_FULL             2




// https://github.com/adafruit/Adafruit_SH110X
// https://github.com/adafruit/Adafruit_SSD1306
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


//Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SH1106G oled = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// we need this for timing if an event occurred which 
// should be visualized with priortity for a certain amount of time
long Output_EventTime = 0;
int Output_Event = 0;
int32_t Output_EventTimeOut = 0;

int Output_Value[32];

int NumberToBeShown = 0;
String TextToBeShown = "";
String TextSecondLineToBeShown = "";
String ThirdLineToBeShown = "";

uint8_t DisplayMode = 0;

#include "fields.h"

#define  P_ANY         255
#define  C_TUNING           1

#define  P_VOLUME      1
#define  C_INPUT            3
#define  C_VOLUME           2

#define  P_MAXVALUE    2
#define  C_INPUT2           4
#define  C_MAXVALUE         5

#define  P_NUMBER      3
#define  C_NUMBER           6
#define  C_FIRMWARE         7

#define  P_PARAM       4
#define  C_PARAM            8


void Output_SetFields()
{
  FieldSet(C_TUNING,   P_ANY,      32, 2,  40, 20, "", 1, 2);
  
  FieldSet(C_VOLUME,   P_VOLUME,   17, 22, 40, 20, "", 1, 3);
  FieldSet(C_INPUT,    P_VOLUME,   32, 50, 40, 20, "", 1, 2);
  
  FieldSet(C_MAXVALUE, P_MAXVALUE, 17, 22, 40, 20, "", 1, 3);
  FieldSet(C_INPUT2,   P_MAXVALUE, 32, 50, 40, 20, "", 1, 2);
 
  FieldSet(C_NUMBER,   P_NUMBER,   17, 22, 40, 20, "#" + String( Output_Value[OCHAN_DEVICENUMBER] ), 1, 3);
  FieldSet(C_FIRMWARE, P_NUMBER,   32, 50, 40, 20, FirmwareVersion, 1, 2);
  
  FieldSet(C_PARAM,    P_PARAM,    17, 22, 40, 20, "", 1, 3);
}




// https://github.com/adafruit/Adafruit_SH110x/blob/master/examples/OLED_QTPY_SH1106/SH1106_128x64_i2c_QTPY/SH1106_128x64_i2c_QTPY.ino

// // this is called once at the beginning, to initiate all visualizing elements
void Output_Init()
{
  // in that case, we initiate a LED which is active high
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW); 
  
  if ( !oled.begin(SCREEN_ADDRESS, true) ) 
  { 
    Serial.println(F("OLED allocation failed")); 
  }
  else
  {
    oled.display();
    //oled.setRotation(1);
    delay(50); // Pause

    // Clear the buffer
    oled.clearDisplay();

    Serial.println(F("OLED I2C Access"));
  }

  Output_SetFields();
};



// set the value of a certain channel to be visualized
void Output_SetValue(int Channel, int Value)
{
  Output_Value[Channel] = Value;
}


// set the value of a certain channel to be visualized
void Output_SetDisplayMode(int Mode)
{
  DisplayMode = Mode;
}


int Output_GetValue(int Channel)
{
  return Output_Value[Channel];
}



void Display_ShowNumberP(String text, uint16_t Number, String SecondLineText, String ThirdLineText)
{
  FieldSetText(C_PARAM, text + " " + String(Number) );
  
  PageChange(P_PARAM); 
 
}


// call when an event occurred which should be visualized with priority for a certain amount of time
void Output_ShowEvent(int intEvent, int EventTimeOut)
{
  Output_EventTime = millis();
  Output_Event = intEvent;
  Output_EventTimeOut = EventTimeOut;
   
  if (intEvent == EVENT_MESSAGE_RECEIVED) digitalWrite(PIN_LED, HIGH); 
  
  // if (intEvent == EVENT_SHOW_RADIO1)  Display_ShowNumber("R", 2 * Output_Value[ OCHAN_RADIO1_INDEX ] + 1,"",""); 
  // if (intEvent == EVENT_SHOW_RADIO2)  Display_ShowNumber("R", 2 * Output_Value[ OCHAN_RADIO1_INDEX ],"",""); 

  if (intEvent == EVENT_SHOW_INPUT)        Display_ShowNumberP("I", Output_Value[OCHAN_INPUT],"",""); 
  if (intEvent == EVENT_SHOW_BASSBALANCE)  Display_ShowNumberP("B", Output_Value[OCHAN_BASSBALANCE],"",""); 
  if (intEvent == EVENT_SHOW_LOWPASS)      Display_ShowNumberP("L", Output_Value[OCHAN_LOWPASS],"",""); 
  if (intEvent == EVENT_SHOW_HIGHPASS)     Display_ShowNumberP("H", Output_Value[OCHAN_HIGHPASS],"",""); 
  if (intEvent == EVENT_SHOW_PARAMETRIC)   Display_ShowNumberP("P", Output_Value[OCHAN_PARAMETRIC],"",""); 
  if (intEvent == EVENT_SHOW_FILTERBANK)   Display_ShowNumberP("F", Output_Value[OCHAN_FILTERBANK],"",""); 
  if (intEvent == EVENT_SHOW_DELAY)        Display_ShowNumberP("D", Output_Value[OCHAN_DELAY],"",""); 
  if (intEvent == EVENT_SHOW_COMPRESSOR)   Display_ShowNumberP("C", Output_Value[OCHAN_COMPRESSOR],"","");
  if (intEvent == EVENT_SHOW_STEREOWIDTH)  Display_ShowNumberP("S", Output_Value[OCHAN_STEREOWIDTH],"","");
  if (intEvent == EVENT_SHOW_ROUTING)      Display_ShowNumberP("R", Output_Value[OCHAN_ROUTING],"","");
 
}



// call this regularly to check if the time has ended for the event to be visualized
void Output_EventLoop()
{
  if (Output_Event != 0)
  {
    if ((millis() - Output_EventTime) > Output_EventTimeOut)
    {
        if (Output_Event == EVENT_MESSAGE_RECEIVED) 
        {
          digitalWrite(PIN_LED, LOW);
        }
        else
        {
          DisplayMode = 0;
          PageChange( P_VOLUME ); 
        }
        Output_Event = 0;
    }
  }
}


// call this regularly to visulize all the values and check if the time has ended for the event to be visualized
void Output_Refresh(uint8_t RefreshType)
{
 
  if (Output_Value[OCHAN_DATAFLOW] == 0)
  {
    digitalWrite(PIN_LED, HIGH); 
  }
  else
  {
    if (Output_Event != EVENT_MESSAGE_RECEIVED) digitalWrite(PIN_LED, LOW); 
  }   
  
  if (RefreshType == REFRESHTYPE_FULL)
  {
    Output_EventLoop();

    if (Output_Event == 0)
    {
      if (Output_Value[OCHAN_ERROR] == 0)
      {
          FieldSetText( C_TUNING, "" );
          if ( Output_Value[OCHAN_TUNING] == 2 ) FieldSetText( C_TUNING, "TUNING" );

          if ( Output_Value[OCHAN_INPUT] == SIGNAL_AIR )          FieldSetText( C_INPUT, "Air" ); 
          if ( Output_Value[OCHAN_INPUT] == SIGNAL_DIGITAL )      FieldSetText( C_INPUT, "RS485" ); 
          if ( Output_Value[OCHAN_INPUT] == SIGNAL_TEST_MONO )    FieldSetText( C_INPUT, "Test_M" ); 
          if ( Output_Value[OCHAN_INPUT] == SIGNAL_TEST_STEREO )  FieldSetText( C_INPUT, "Test_S" ); 

          FieldSetText( C_INPUT2, FieldGetText( C_INPUT ) );

          FieldSetText( C_VOLUME,   "V " + String( Output_Value[OCHAN_LEVEL_VOLUME1] ) );
          FieldSetText( C_MAXVALUE, String( Output_Value[OCHAN_MAXVALUE_LEFT] ) + "/" + String( Output_Value[OCHAN_MAXVALUE_RIGHT] ) );
          FieldSetText( C_NUMBER,   "#" + String( Output_Value[OCHAN_DEVICENUMBER] ) );

          if (DisplayMode == 0)
          { 
            PageChange( P_VOLUME );
          }

          if (DisplayMode == 1)
          {
            PageChange( P_MAXVALUE );
          }

          if (DisplayMode == 2)
          {
           PageChange( P_NUMBER  );
          }
      }
      else
      {
        // TO DO
      }        
    }
  }

}



void Output_ShowValue(int Channel, int Value)
{
  uint8_t RefreshType = REFRESHTYPE_FULL;
  
  Output_Value[Channel] = Value;

  if ( (Channel == OCHAN_DATAFLOW) ) RefreshType = REFRESHTYPE_LEDONLY;
  Output_Refresh(RefreshType);
}

