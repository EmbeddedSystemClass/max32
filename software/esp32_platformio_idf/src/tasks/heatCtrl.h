#ifndef HEATCTRL_H
#define HEATCTRL_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

void heatController_task( void* pvParameters );

bool setTemperature( float temperature );

#ifdef __cplusplus
}
#endif

#endif //HEATCTRL_H