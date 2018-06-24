#ifndef WLAN_H
#define WLAN_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define WIFI_CONNECTED_BIT 0x01

#ifdef __cplusplus
extern "C" {
#endif

//EventGroup for wifi event distribution
extern EventGroupHandle_t wifi_event_group;

void wlan_init();

uint64_t wlan_get_mac_lsb_first();

void wlan_sleep();

#ifdef __cplusplus
}
#endif


#endif //WLAN_H