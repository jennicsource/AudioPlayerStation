/*
 
Nordic from top, Antenna to the right

GND  VCC         brown red 
CE   CS          green violet
SCK  MOSI        blue  yellow 
MISO IR          orange      
*/

/*

// this is for ESP32-S3 Audio Transceiver Board

#define pCE        9      // green
#define pCS        10     // violet
#define pSCK       12     // blue
#define pMOSI      11     // yellow   
#define pMISO      13     // orange

#define pCE2        0    // 
#define pCS2        0    // 
#define pSCK2       12    // 
#define pMOSI2      11    // 
#define pMISO2      13    // 

*/

// http://wiki.fluidnc.com/en/hardware/ESP32-S3_Pin_Reference
// https://user-images.githubusercontent.com/13697838/230209347-d2b07681-5a45-4b29-bb78-7eb0c7f4c498.png

// https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
// https://wiki.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-embedded-battery-management-chip/


// this is for the ESP32-S3 as XIAO board

#define pCE        3     // 
#define pCS        4     // 
#define pSCK       7     // 
#define pMOSI      9     // 
#define pMISO      8     // 

#define pCE2        0    // 
#define pCS2        0    // 
#define pSCK2       0    // 
#define pMOSI2      0    // 
#define pMISO2      0    // 

#define pCE3        0    //
#define pCS3        0    // 
#define pSCK3       0    // 
#define pMOSI3      0    // 
#define pMISO3      0    // 



#define PIN_SWITCH    1   // = D0

// Audio Transceiver Board 
/*
#define UART_RX       18
#define UART_TX       17
*/

// XIAO Board
#define UART_RX       44
#define UART_TX       43
