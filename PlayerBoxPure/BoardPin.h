/*
 
Nordic from top, Antenna to the right

GND  VCC         brown red 
CE   CS          green violet
SCK  MOSI        blue  yellow 
MISO IR          orange      
*/


// http://wiki.fluidnc.com/en/hardware/ESP32-S3_Pin_Reference
// https://user-images.githubusercontent.com/13697838/230209347-d2b07681-5a45-4b29-bb78-7eb0c7f4c498.png

// https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
// https://wiki.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-embedded-battery-management-chip/


// pin definitions for up to 3 radio modules
#define pCE        9     
#define pCS        10    
#define pSCK       12    
#define pMOSI      11       
#define pMISO      13    
#define pIRQ       14    // IRQ pin

#define pCE2        4    
#define pCS2        5    
#define pSCK2       12   
#define pMOSI2      11    
#define pMISO2      13    

// this board has just 2 radio modules
#define pCE3        0    
#define pCS3        0    
#define pSCK3       0   
#define pMOSI3      0    
#define pMISO3      0   

// pin definitions for I2S / ADC
#define I2S_MCK_IN  37  //  blue
#define I2S_WS_IN   38  //  green
#define I2S_SCK_IN  39  //  yellow
#define I2S_SD_IN   40  //  orange 
  
// pin definitions for I2S / DAC
#define I2S_MCK_OUT 36  //  blue
#define I2S_WS_OUT  35  //  green
#define I2S_SCK_OUT 48  //  yellow
#define I2S_SD_OUT  47  //  orange 


// the hardware UART for fast transmission
#define UART_RX       18
#define UART_TX       17

// the pins for the I2C bus
#define I2C_SDA       16
#define I2C_SCL       15



// this is for the XIAO ESP32-S3 board
/*
#define UART_RX       44
#define UART_TX       43
*/
