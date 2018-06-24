#ifndef SI7020_H
#define SI7020_H

#include <stdint.h>

class SI7020 {

private:
  uint8_t m_address;
  void readRegister(uint8_t reg, uint8_t& dest, uint8_t len);
  
public:
  static const uint8_t SERIAL_NUMBER = 0x14;

  SI7020(uint8_t Address);
  char getSerial(void);
  float getTemperature(void);
  float getHumidity(void);
};

#ifdef __cplusplus
extern "C" {
#endif

extern void si7020_read(uint8_t address, uint8_t* dst, uint32_t length);
extern void si7020_write(uint8_t address, uint8_t* src, uint32_t length, bool stop);
extern void si7020_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif //SI7020_H
