#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Wifi default Settings */
#define WIFI_SSID "SSID" //WiFI SSID
#define WIFI_PASSWORD "PASSWORD" //Wifi password

/* MQTT Settings */
#define MQTT_BROKER  "192.168.0.4"
#define MQTT_PORT  "1883"
#define MQTT_USERNAME  ""
#define MQTT_PASSWORD  ""
#define MQTT_SUBSCRIPTION_PREFIX  "max32/cmd/"
#define MQTT_PUBLICATION_PREFIX  "max32/status/"

#ifdef __cplusplus
}
#endif

#endif //CONFIGFILE_H