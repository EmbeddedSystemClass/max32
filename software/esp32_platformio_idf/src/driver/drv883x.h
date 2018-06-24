#ifndef DRV883X_H
#define DRV883X_H

#include <stdint.h>

class DRV883X {

private:
  uint8_t pinEnable = 0;
  uint8_t pinPhase = 0;
  uint8_t pinSleep = 0;

public:
  DRV883X(uint8_t enablePin, uint8_t phasePin, uint8_t sleepPin);
  void sleep(bool sleep);
  void enable(bool en);
  void direction(uint8_t dir);
  void start( bool dir );
  void stop();


};

#endif //DRV883X_H
