

//XIAO ESP32 C3       https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/
//                    https://wiki.seeedstudio.com/xiao_esp32s3_pin_multiplexing/
//XIAO ESP32 S3       https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
// https://www.seeedstudio.com/Grove-Shield-for-Seeeduino-XIAO-p-4621.html


/*

C3

D0 GPIO2/LEDBAR
D1 GPIO3/LEDBAR
D2 GPIO4/TEMP/A2
D3   CS             D10   g2 MOSI
D4  I2C             D9    g  MISO
D5  I2C             D8    w  SCK/RELAY
D6   TX             D7   GPIO20/RX/IR



S3

D0 GPIO1/LEDBAR
D1 GPIO2/LEDBAR
D2 GPIO3/TEMP/A2
D3   CS             D10   g2 MOSI
D4  I2C             D9    g  MISO
D5  I2C             D8    w  SCK/RELAY
D6   TX  GPIO 43    D7   GPIO44/RX/IR

*/



// C3
/*
#define PIN_LEDBAR_CLOCK   2
#define PIN_LEDBAR_DATA    3

#define PIN_I2C_SDA    6
#define PIN_I2C_SCL    7

#define PIN_TEMP    4

#define PIN_IR     20 

#define PIN_RELAY       8

#define SD_CS_PIN        5
#define SD_CLK_PIN       8
#define SD_MOSI_PIN      10
#define SD_MISO_PIN      9


#define UART_RX       20
#define UART_TX       21
*/

// S3

#define PIN_LEDBAR_CLOCK   1
#define PIN_LEDBAR_DATA    2

#define PIN_I2C_SDA    5
#define PIN_I2C_SCL    6

#define PIN_ADC    A2

#define PIN_IR     44

#define PIN_RELAY       7

#define SD_CS_PIN        4
#define SD_CLK_PIN       7
#define SD_MOSI_PIN      9
#define SD_MISO_PIN      8

#define UART_RX       44
#define UART_TX       43


/*
// S3 Dev Kit

#define PIN_LEDBAR_CLOCK   18
#define PIN_LEDBAR_DATA    17

#define PIN_I2C_SDA    4
#define PIN_I2C_SCL    5

#define PIN_IR     15


#define UART_RX       7
#define UART_TX       6

*/
