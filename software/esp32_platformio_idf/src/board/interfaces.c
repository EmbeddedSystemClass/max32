#include "interfaces.h"
#include <stdint.h>
#include <stdbool.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "board/board.h"
//#include "driver/si7020.h"

#define SI7020_ACK  0x00
#define SI7020_NACK 0x01 

/* Generic interface functions */
void i2c_init() {

    //Configure I2C interface as master
    i2c_driver_install( THSPort, I2C_MODE_MASTER, 0, 0, 0);

}

/* SI7020 interface definition */

void si7020_read(uint8_t address, uint8_t* dst, uint32_t length) {
    //Command queue
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();

    //Send start command
    i2c_master_start( cmd );

    //Send slave address
    i2c_master_write_byte( cmd, (address<<1 | I2C_MASTER_READ), true );

    if( length > 1) {
        //Read Data
        i2c_master_read( cmd, dst, length-1, SI7020_ACK );
    }
    //Read last byte with NACK
    i2c_master_read_byte( cmd, &dst[length-1], SI7020_NACK );

    //Stop read
    i2c_master_stop( cmd );

    //execute cmd
    int err;
    err = i2c_master_cmd_begin( THSPort, cmd,  DELAY_MS(50) );

    //Delete cmd
    i2c_cmd_link_delete( cmd );

    if(err != ESP_OK) {
        ESP_LOGE( "SI7020", "Read i2c err = 0x%x", err );
    }
}

void si7020_write(uint8_t address, uint8_t* src, uint32_t length, bool stop) {
    //Command queue
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();

    //Send start command
    i2c_master_start( cmd );

    //Send slave address
    i2c_master_write_byte( cmd, (address<<1 | I2C_MASTER_WRITE), true );

    //Write Data
    for(int i=0; i<length; i++)
        i2c_master_write_byte( cmd, src[i], true );

    //Stop writing
    if(stop)
        i2c_master_stop( cmd );

    //execute cmd
    int err;
    err = i2c_master_cmd_begin( THSPort, cmd,  DELAY_MS(50) );

    //Delete cmd
    i2c_cmd_link_delete( cmd );

    if(err != ESP_OK) {
        ESP_LOGE( "SI7020", "Write i2c err = 0x%x", err );
    }
}

void si7020_delay_ms(uint32_t ms) {
    vTaskDelay(DELAY_MS(ms));
}