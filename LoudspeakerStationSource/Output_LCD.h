
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


//#include "SevenSegmentDisplay.h"

// https://docs.freenove.com/projects/fnk0079/en/latest/fnk0079/codes/ESP32/ESP32_C/1_LCD1602.html

long Output_EventTime = 0;
int Output_Event = 0;
int32_t Output_EventTimeOut = 0;

int Output_Value[12];

int NumberToBeShown = 0;

//#include <LiquidCrystal_I2C.h>

#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Clcd.h> // i2c expander i/o class header
hd44780_I2Clcd lcd(0x27); // declare lcd object: auto locate & auto config expander chip

//LiquidCrystal_I2C lcd(0x27,16,2);  // 0x3F

void Output_Init()
{
  //SevenSegmentDisplay_Init(PIN_LEDBAR_CLOCK, PIN_LEDBAR_DATA);

/*
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcd.setCursor(1,1);             // Move the cursor to row 0, column 0
  lcd.print("hello!");     // The print content is displayed on the LCD
*/

  int status = lcd.begin(16, 2);
  Serial.println(status);
	if(status) // non zero status means it was unsuccesful
	{
		// hd44780 has a fatalError() routine that blinks an led if possible
		// begin() failed so blink error code using the onboard LED if possible
		hd44780::fatalError(status); // does not return
	}

	// initalization was successful, the backlight should be on now
  lcd.setBacklight(255);
  //lcd.display();
  lcd.clear();
	// Print a message to the LCD
	lcd.print("Hello, World!");
  lcd.display();

  Serial.println("Display done");
};




void Scan_I2C()
{

  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }


}




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
