#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "board.h"

/* IO Configuration */

//LEDs
#if BOARD_VERSION == 1

const gpio_config_t ledConfig = {
    .pin_bit_mask = ((1ULL << LED_RED) | (1ULL << LED_GREEN) | (1ULL << LED_BLUE)),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

#elif BOARD_VERSION == 2

const gpio_config_t ledConfig = {
    .pin_bit_mask = ( 1ULL << LED ),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

//Display
const gpio_config_t displayPinConfig = {
    .pin_bit_mask = ( 1ULL << DISP_DC ),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};
const spi_host_device_t displaySPIHost = HSPI_HOST;

const spi_bus_config_t displaySPIConfig = {
    .mosi_io_num = SPI_MOSI,
    .miso_io_num = -1, //-1 for not used
    .sclk_io_num = SPI_CLK,
    .quadwp_io_num = -1, //-1 for not used
    .quadhd_io_num = -1, //-1 for not used
    .max_transfer_sz = 0 //Default max size
};

#endif

//Switches
const gpio_config_t btnConfig = {
    .pin_bit_mask = ((1ULL << BUTTON_1) | (1ULL << BUTTON_2) | (1ULL << BUTTON_3)),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

//Motor driver
const gpio_config_t drv8838Config = {
    .pin_bit_mask = ((1ULL << MOTOR_ENABLE) | (1ULL << MOTOR_nSLEEP) | (1ULL << MOTOR_PHASE)),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

//Reflex coupler tx diode
const gpio_config_t reflexCtrlConfig = {
    .pin_bit_mask = (1ULL << REFLEX_EN),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

//Reflex coupler rx diode
const gpio_config_t reflexSigConfig = {
    .pin_bit_mask = (1ULL << REFLEX_SIG),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE, //.pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

//Temp. & Humidity I2C
const i2c_port_t THSPort = I2C_NUM_0;
const i2c_config_t sensorConfig = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = SENS_SDA,
    .sda_pullup_en = GPIO_PULLUP_ENABLE, //GPIO_PULLUP_DISABLE
    .scl_io_num = SENS_SCL,
    .scl_pullup_en = GPIO_PULLUP_ENABLE, //GPIO_PULLUP_DISABLE,
    .master.clk_speed = 100000
};

//UART
const uart_port_t SERIALPort = UART_NUM_0;
const uart_config_t uartConfig = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
};

//SD-Card

/* Functions */

int board_init(void)
{
    //LED pins
    gpio_config( &ledConfig );
    
    //Button pins
    gpio_config( &btnConfig );
    
    //Motor pins
    gpio_config( &drv8838Config );
    
    //Refelx coupler
    gpio_config( &reflexCtrlConfig );
    gpio_config( &reflexSigConfig );

#if BOARD_VERSION <= 1
#warning Board 1.1 contains a error in display connection. The usage of the display is not possible and will cause in a failed boot of the esp32
#else
    //Display
    gpio_config( &displayPinConfig );
    bool isNative;
    spi_bus_initialize( displaySPIHost, &displaySPIConfig, 0);
    spicommon_bus_initialize_io( displaySPIHost, &displaySPIConfig, 0, SPICOMMON_BUSFLAG_MASTER, &isNative);
    spicommon_cs_initialize( displaySPIHost, SPI_CS, 0, false);
#endif
    
    //Temp. & Humidity Sensor
    i2c_param_config( THSPort, &sensorConfig );

    //UART - init the uart is not necessary if printf is used
    // uart_param_config( SERIALPort, &uartConfig );
    // uart_set_pin( SERIALPort, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE );

    return 0;
}

#if BOARD_VERSION == 1

void board_setLedRed(bool state)
{
    gpio_set_level( LED_RED, state);
}

void board_setLedGreen(bool state)
{
    gpio_set_level( LED_GREEN, state);
}

void board_setLedBlue(bool state)
{
    gpio_set_level( LED_BLUE, state);
}

#elif BOARD_VERSION == 2

void board_setLed(bool state)
{
    gpio_set_level( LED, state);
}

#endif

void board_setReflexEn(bool state)
{
    gpio_set_level( REFLEX_EN, state);
}

bool  board_getReflexEn()
{
    return gpio_get_level( REFLEX_EN ) > 0;
}

bool board_getReflexSig()
{
    return gpio_get_level( REFLEX_SIG ) > 0;
}
