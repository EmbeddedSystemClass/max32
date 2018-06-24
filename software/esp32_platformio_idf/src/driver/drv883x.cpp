#include "driver/gpio.h"
#include "drv883x.h"

DRV883X::DRV883X(uint8_t enablePin, uint8_t phasePin, uint8_t sleepPin)
{
  pinEnable = enablePin;
  pinPhase = phasePin;
  pinSleep = sleepPin;
}

void DRV883X::sleep(bool sleep)
{
  gpio_set_level( (gpio_num_t)pinSleep, !sleep );
}

void DRV883X::enable(bool en)
{
  gpio_set_level( (gpio_num_t)pinEnable, en );
}

void DRV883X::direction(uint8_t dir)
{
  gpio_set_level( (gpio_num_t)pinPhase, dir>0 );
}

//dir = direction: false = forward, true = reverse
void DRV883X::start( bool dir )
{
  sleep(false);
  direction(dir);
  enable(true);
}

void DRV883X::stop()
{
  enable(false);
  sleep(true);
  direction(0); //Set low for minimum current
}