#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/* Topic Hierachie:

toplevelname/function/item...

e.g.:
knxgateway1/status/Kitchen/Lights/Front Left

*/

#define TOPIC_TEMPERATURE "temperature"
#define TOPIC_HUMIDITY "humidity"
#define TOPIC_VALVE "valve"
#define TOPIC_BATTERY "battery"

#define MQTT_CONNECTED_BIT 0x01

#ifdef __cplusplus
extern "C" {
#endif

extern EventGroupHandle_t wifi_event_group;
extern EventGroupHandle_t mqtt_event_group;

void mqttClient_init(const char* host, const char* port, const char* username, const char* password, const char* pub_topic_prefix, const char* sub_topic_prefix);

void mqttClient_task(void* pvParameters);
    
void mqttClient_pubTemperature(float temperature);

void mqttClient_pubHumidity(float humidity);

void mqttClient_pubValve(uint8_t percent);

void mqttClient_pubBattery(float voltage);

#ifdef __cplusplus
}
#endif

#endif //MQTTCLIENT_H