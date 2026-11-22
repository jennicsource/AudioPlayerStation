

//  https://wiki.seeedstudio.com/xiao_nrf54l15_sense_getting_started/

//  https://wiki.seeedstudio.com/xiao_nrf54l15_sense_arduino/

//  https://github.com/lolren/nrf54-arduino-core

//  https://www.seeedstudio.com/XIAO-nRF54L15-Sense-p-6494.html


/*
D0  1.04          5V
D1  1.05          GND 
D2  1.06          VCC
D3  1.07          D10  2.02
D4  1.10          D9   2.04 
D5  1.11          D8   2.01
D6  2.08          D7   2.07
*/



#define SCK_PIN   36   // P1.04 (XIAO D0)
#define MISO_PIN  37   // P1.05 (XIAO D1)
#define MOSI_PIN  38   // P1.06 (XIAO D2)

#define CSN_PIN   39   // P1.07 (XIAO D3)


#define RADIO_GOOD_PIN   PIN_D6

#define LED_PIN   LED_BUILTIN  // 