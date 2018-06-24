#include <stdint.h>
#include "si7020.h"

SI7020::SI7020(uint8_t Address)
{
  m_address = Address;
}

char SI7020::getSerial(void)
{
  uint8_t buffer[14];

  //Start data request
  uint8_t regs[2] = {0xFA, 0x0F};
  si7020_write( m_address, regs, 2, false );

  //Readback 1st
  si7020_read( m_address, buffer, 8 );
  
  //2nd Data request
  uint8_t regs2[2] = {0xFC, 0xC9};
  si7020_write( m_address, regs2, 2, false );
  
  //Readback 2nd
  si7020_read( m_address, buffer+8, 6 );

  //Get serial number from SNB_3
  return buffer[8];
}

float SI7020::getTemperature(void)
{
  uint8_t buffer[2];
  
  //Start data request
  uint8_t reg = 0xE3;
  si7020_write( m_address, &reg, 1, false );
  
  //Wait a couple of time till measurement is done
  si7020_delay_ms(20);

  //Readback temperature register
  si7020_read(m_address, buffer, 2);
  
  //Correct MSB and LSB
  uint16_t temp =  ( (uint16_t)buffer[0] )<<8 | buffer[1];

  //Calculate temperature in °C and return it
  return ( (175.72 * ( (float)temp) )/65536 ) - 46.85;  
}

float SI7020::getHumidity(void)
{
  uint8_t buffer[2];
  
  //Start data request
  uint8_t reg = 0xE5;
  si7020_write( m_address, &reg, 1, false );

  //Wait a couple of time till measurement is done
  si7020_delay_ms(20);

  //Readback temperature register
  si7020_read(m_address, buffer, 2);
  
  //Correct MSB and LSB
  uint16_t humidity = ( (uint16_t)buffer[0] )<<8 | buffer[1];

  //Calculate humidity in %RH and return it
  return (125.0f*(float)humidity)/65536.0f - 6.0f;
}

/* Private functions */
void SI7020::readRegister(uint8_t reg, uint8_t& dest, uint8_t len)
{

}
