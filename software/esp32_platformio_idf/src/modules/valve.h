#ifndef VALVE_H
#define VALVE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//Init valve
void valve_init();  
//Set valve poisition 0-100% (0 = closed, 100 = Open), return true ond success, otherwise false
bool valve_set(uint8_t percent);
//Get the actual valve position from 0 to 100%
uint8_t valve_get();
//Init valve by extract and retract the valve till max
bool valve_calibration();
//Get max counts
uint32_t valve_getMaxCounts();

#ifdef __cplusplus
}
#endif

#endif //VALVE_H
