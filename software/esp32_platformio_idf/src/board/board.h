#ifndef BOARD_H
#define BOARD_H

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/uart.h"

#define BOARD_VERSION 2
#define BOARD_MAJOR_VERSION 2
#define BOARD_MINOR_VERSION 0

// SD Card
#define SD_POWER  16
#define SD_CLK    14
#define SD_CMD    15
#define SD_DATA0  2
#define SD_DATA1  4
#define SD_DATA2  12
#define SD_DATA3  13

//Refelx photocell/Rotary encoder
#define REFLEX_EN   17
#define REFLEX_SIG  21

//Motor controller (DRV8838)
#define MOTOR_nSLEEP 5
#define MOTOR_ENABLE 19
#define MOTOR_PHASE  18

//Temperature and humidity sensor (SI7020)
#define SENS_SDA  23
#define SENS_SCL  22

//UART
#define UART_TX 1
#define UART_RX 3

/* Peripheral defines */
//Temperature and humidity sensot (SI7020)
#define SI7020_ADDR 0x40

/* Macros */
#define DELAY_MS(m) ((m/portTICK_RATE_MS))

#if BOARD_VERSION == 1

/* PINMAP 
PIN(CHIP)  FUNCTION     PERIPHERIE    DESCRIPTION
6  (I34)   Digital IN   Button 3
7  (I35)   Digital IN   Button 1
8  (IO32)  Digital OUT  LED Red
9  (IO33)  Digital IN   Button 2
10 (IO25)  Digital OUT  LED Green
11 (IO26)  -            -
12 (IO27)  -            -
13 (IO14)  SD_CLK       SD Card Clock
14 (IO12)  SD_DATA2     SD Card Data#2
16 (IO13)  SD_DATA3     SD Card Data#3
17 (SD2)   DP_DC        Display Data/Command
18 (SD3)   DP_RST       Display Reset
19 (CMD)   SPI_CS       Display SPI Chipselect
20 (CLK)   SPI_CLK      Display SPI Clock
21 (SD0)   SPI_MOSI     Display SPI MOSI
22 (SD1)   -            -
23 (IO15)  SD_CMD       SD Card Command
24 (IO2)   SD_DATA0     SD Card Data#0
25 (IO25)  BOOT         -
26 (IO4)   SD_DATA1     SD Card Data#1
27 (IO16)  Digital OUT  LED Blue/SD Card Power
28 (IO17)  Digital OUT  Reflex Enable (REFLEX_EN)
29 (IO5)   Digital OUT  Motor nSleep
30 (IO18)  Digital OUT  Motor Phase
31 (IO19)  Digital OUT  Motor Enable
32 (IO20)  -            - 
33 (IO21) Digital IN    Reflex Signal (REFLEX_SIG)
34 (RXD0) RXD           UART Receive
35 (TXD0) TXD           UART Transmit
36 (IO22) SCL           SI7020 I²C Clock
37 (IO23) SDA           SI7020 I²C Data
*/

/* Hardware defines */
//Leds
#define LED_RED   32
#define LED_GREEN 25 //LED_GRREN shares pin with display on/off signal
#define LED_BLUE  16 //LED_BLUE shares pin with SD on/off signal

//Userbuttons
#define BUTTON_1  35
#define BUTTON_2  33
#define BUTTON_3  34

//Display
#define DISP_POWER 25 //LED_GRREN shares pin with display on/off signal
#define DISP_DC    9
#define DISP_RST   10
#define SPI_CS     11
#define SPI_CLK    6
#define SPI_MOSI   7

void board_setLedRed(bool state);
void board_setLedGreen(bool state);
void board_setLedBlue(bool state);

//Compatibility wrapper
#define board_setLed(b) (board_setLedRed(b))

#endif

#if BOARD_VERSION == 2

/* PINMAP 
PIN(CHIP)  FUNCTION     PERIPHERIE    DESCRIPTION
4  (I36)   Analog IN    Battery       Batter voltage measurement
5  (I39)   Digital IN   Button 2
6  (I34)   Digital IN   Button 3
7  (I35)   Digital IN   Button 1
8  (IO32)  Digital OUT  User LED & Display Power
9  (IO33)  Digital OUT  Display SPI Chipselect
10 (IO25)  Digital OUT  Display SPI Data/Command
11 (IO26)  SPI_MOSI     Display SPI MOSI
12 (IO27)  SPI_CLK      Display SPI Clock
13 (IO14)  SD_CLK       SD Card Clock
14 (IO12)  SD_DATA2     SD Card Data#2
16 (IO13)  SD_DATA3     SD Card Data#3
17 (SD2)   DP_DC        Display Data/Command
18 (SD3)   DP_RST       Display Reset
19 (CMD)   -            -
20 (CLK)   -            -
21 (SD0)   -            -
22 (SD1)   -            -
23 (IO15)  SD_CMD       SD Card Command
24 (IO2)   SD_DATA0     SD Card Data#0
25 (IO25)  BOOT         -
26 (IO4)   SD_DATA1     SD Card Data#1
27 (IO16)  Digital OUT  SD Card Power (SD_POWER)
28 (IO17)  Digital OUT  Reflex Enable (REFLEX_EN)
29 (IO5)   Digital OUT  Motor nSleep
30 (IO18)  Digital OUT  Motor Phase
31 (IO19)  Digital OUT  Motor Enable
32 (IO20)  -            - 
33 (IO21) Digital IN    Reflex Signal (REFLEX_SIG)
34 (RXD0) RXD           UART Receive
35 (TXD0) TXD           UART Transmit
36 (IO22) SCL           SI7020 I²C Clock
37 (IO23) SDA           SI7020 I²C Data
*/

/* Hardware defines */
//Leds
#define LED       32

//Userbuttons
#define BUTTON_1  35
#define BUTTON_2  39
#define BUTTON_3  34

//Display
#define DISP_POWER 32 //Shared with LED
#define DISP_DC    25
#define SPI_CS     33
#define SPI_CLK    27
#define SPI_MOSI   26

#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Global interface instances */

//Temperature and Humidity Sensor
extern const i2c_port_t THSPort;

//UART Serial interface
extern const uart_port_t SERIALPort;

/* Prototypes */

//Init all port pins and ports
int board_init(void);

//Setter function for led true = on, false = off
void board_setLed(bool state);

//Setter function for reflex enable pin
void board_setReflexEn(bool state);
//Getter function for reflex enable pin
bool  board_getReflexEn();
//Getter function for refelx input pin
bool board_getReflexSig();

#ifdef __cplusplus
}
#endif

#endif //BOARD_H