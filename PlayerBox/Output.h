
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

int Output_Value[24];

int NumberToBeShown = 0;
String TextToBeShown = "";
String TextSecondLineToBeShown = "";

uint8_t DisplayMode = 0;


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

    Serial.println(F("I2C Access"));

  }
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


void Display_ShowNumber(String text, uint16_t Number, String SecondLineText)
{
  oled.clearDisplay();
  oled.setTextSize(3); // 
  oled.setTextColor(SH110X_WHITE);
  oled.setCursor(17, 22);
  oled.println(text + String(Number));

  oled.setTextSize(2); // 
  oled.setTextColor(SH110X_WHITE);
  oled.setCursor(32, 50);
  oled.println(SecondLineText);

  oled.display();      // Show initial text
}


// call when an event occurred which should be visualized with priority for a certain amount of time
void Output_ShowEvent(int intEvent, int EventTimeOut)
{
  Output_EventTime = millis();
  Output_Event = intEvent;
  Output_EventTimeOut = EventTimeOut;
  
  if (intEvent == EVENT_MESSAGE_RECEIVED) Display_ShowNumber("",8888,""); 
  if (intEvent == EVENT_SHOW_AUDIO_PARAM)  Display_ShowNumber("AP", Output_Value[OCHAN_AUDIO_PARAM],""); 
  if (intEvent == EVENT_SHOW_RADIO1)  Display_ShowNumber("R", 2 * Output_Value[ OCHAN_RADIO1_INDEX ] + 1,""); 
  if (intEvent == EVENT_SHOW_RADIO2)  Display_ShowNumber("R", 2 * Output_Value[ OCHAN_RADIO1_INDEX ],""); 
  if (intEvent == EVENT_SHOW_INPUT)  Display_ShowNumber("I", Output_Value[OCHAN_INPUT],""); 
  if (intEvent == EVENT_SHOW_BASSBALANCE)  Display_ShowNumber("B", Output_Value[OCHAN_BASSBALANCE],""); 
  if (intEvent == EVENT_SHOW_LOWPASS)  Display_ShowNumber("L", Output_Value[OCHAN_LOWPASS],""); 
  if (intEvent == EVENT_SHOW_HIGHPASS)  Display_ShowNumber("H", Output_Value[OCHAN_HIGHPASS],""); 
  if (intEvent == EVENT_SHOW_PARAMETRIC)  Display_ShowNumber("P", Output_Value[OCHAN_PARAMETRIC],""); 
  if (intEvent == EVENT_SHOW_FILTERBANK)  Display_ShowNumber("F", Output_Value[OCHAN_FILTERBANK],""); 
 
}



// call this regularly to check if the time has ended for the event to be visualized
void Output_EventLoop()
{
  if (Output_Event != 0)
  {
    if ((millis() - Output_EventTime) > Output_EventTimeOut)
    {
        Output_Event = 0;
        Display_ShowNumber(TextToBeShown, NumberToBeShown, TextSecondLineToBeShown); 
    }
  }
}


// call this regularly to visulize all the values and check if the time has ended for the event to be visualized
void Output_Refresh(uint8_t RefreshType)
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
    if (Output_Event != 0)
    {
      if ((millis() - Output_EventTime) > 700)
      {
          Output_Event = 0;
          Display_ShowNumber(TextToBeShown, NumberToBeShown, TextSecondLineToBeShown); 
      }
    }
    else
    {

      if (Output_Value[OCHAN_ERROR] == 0)
      {
       
          if (DisplayMode == 0)
          {
            NumberToBeShown = Output_Value[OCHAN_LEVEL_VOLUME1];
            TextToBeShown = "V";
            if ( Output_Value[OCHAN_INPUT] == SIGNAL_AIR )     TextSecondLineToBeShown = "Air";
            if ( Output_Value[OCHAN_INPUT] == SIGNAL_DIGITAL ) TextSecondLineToBeShown = "RS485";
            if ( Output_Value[OCHAN_INPUT] == SIGNAL_TEST_MONO ) TextSecondLineToBeShown = "TEST_M";
            if ( Output_Value[OCHAN_INPUT] == SIGNAL_TEST_STEREO ) TextSecondLineToBeShown = "TEST_S";
            if ( Output_Value[OCHAN_TUNING] == 2 ) TextSecondLineToBeShown = "T-" + TextSecondLineToBeShown;
            Display_ShowNumber(TextToBeShown, NumberToBeShown, TextSecondLineToBeShown); 
          }
          if (DisplayMode == 1)
          {
            NumberToBeShown = Output_Value[OCHAN_MAXVALUE_RIGHT];
            TextToBeShown = String( Output_Value[OCHAN_MAXVALUE_LEFT] ) + "/";
            if ( Output_Value[OCHAN_INPUT] == SIGNAL_AIR )     TextSecondLineToBeShown = "Air";
            if ( Output_Value[OCHAN_INPUT] == SIGNAL_DIGITAL ) TextSecondLineToBeShown = "RS485";
            if ( Output_Value[OCHAN_INPUT] == SIGNAL_TEST_MONO ) TextSecondLineToBeShown = "TEST_M";
            if ( Output_Value[OCHAN_INPUT] == SIGNAL_TEST_STEREO ) TextSecondLineToBeShown = "TEST_S";
            if ( Output_Value[OCHAN_TUNING] == 2 ) TextSecondLineToBeShown = "T-" + TextSecondLineToBeShown;
            Display_ShowNumber(TextToBeShown, NumberToBeShown, TextSecondLineToBeShown); 
          }
          if (DisplayMode == 2)
          {
            NumberToBeShown = Output_Value[OCHAN_DEVICENUMBER];
            TextToBeShown = "#";
            TextSecondLineToBeShown = FirmwareVersion;
            Display_ShowNumber(TextToBeShown, NumberToBeShown, TextSecondLineToBeShown); 
          }
       
        
      }
      else
      {
        NumberToBeShown = 9900 + Output_Value[OCHAN_ERROR];
        TextToBeShown = "";
        Display_ShowNumber(TextToBeShown, NumberToBeShown, TextSecondLineToBeShown); 
      }        
    }
  }

}



void Output_ShowValue(int Channel, int Value)
{
  uint8_t RefreshType = REFRESHTYPE_FULL;
  
  Output_Value[Channel] = Value;

  if ( (Channel == OCHAN_ACTIVE_MODULE) ) RefreshType = REFRESHTYPE_LEDONLY;
  Output_Refresh(RefreshType);
}

String Output_EjectVirtualDisplay()
{
  return "-" ;
}
