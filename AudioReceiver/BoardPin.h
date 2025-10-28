/*



Nordic from top, Antenna to the right

GND  VCC      
CE   CS         green violet
SCK  MOSI       blue  yellow
MISO IR         orange

*/



// Digital I/O used


#define pCE        9     // green
#define pCS        10    // violet
#define pSCK       12    // blue
#define pMOSI      11    // yellow    
#define pMISO      13    // orange

#define pIRQ      14    // IRQ pin



#define pCE2        4    // weiß  green
#define pCS2        5    // grau  white
#define pSCK2       12    // lila  blue
#define pMOSI2      11    // blau  yellow 
#define pMISO2      13    // grün  brown 
              

// http://wiki.fluidnc.com/en/hardware/ESP32-S3_Pin_Reference
// https://user-images.githubusercontent.com/13697838/230209347-d2b07681-5a45-4b29-bb78-7eb0c7f4c498.png



#define I2S_MCK_IN  37  //  blue
#define I2S_WS_IN   38  //  green
#define I2S_SCK_IN  39  //  yellow
#define I2S_SD_IN   40  //  orange 
  
#define I2S_MCK_OUT 36  //  blue
#define I2S_WS_OUT  35  //  green
#define I2S_SCK_OUT 48  //  yellow
#define I2S_SD_OUT  47  //  orange 


#define PIN_BUTTON  16  //   SDA
#define PIN_LED  15   //    SCL

//#define PIN_TASK_LED  15   //    

//#define PIN_LEDDISPLAY_CLOCK 15
//#define PIN_LEDDISPLAY_DATA  16

//#define PIN_LED_DIRTY  19 

#define UART_RX       18
#define UART_TX       17

#define UART_RX_2      6  // TX and RX swapped
#define UART_TX_2      7  // TX and RX swapped
  
