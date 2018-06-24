#ifndef INTERFACES_H
#define INTERFACES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Generic interface functions */
void i2c_init();


/* SI7020 I2C Interface definition */
void si7020_read(uint8_t address, uint8_t* dst, uint32_t length);
void si7020_write(uint8_t address, uint8_t* src, uint32_t length, bool stop);
void si7020_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif //INTERFACES_H