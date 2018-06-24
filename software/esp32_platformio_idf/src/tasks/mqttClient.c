#include "mqttClient.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "modules/wlan.h"
#include "modules/valve.h"
#include "tasks/heatCtrl.h"
#include "board/board.h"

/* ESP MQTT C++ Directive */
#ifdef __cplusplus
extern "C" {
#endif

#include "esp-mqtt/esp_mqtt.h"

#ifdef __cplusplus
}
#endif

static const char* pHost;
static const char* pPort;
static const char* pUsername;
static const char* pPassword;
static const char* pSubTopic;
static const char* pPubTopic;
static char pClientId[13];

static SemaphoreHandle_t mqttSemaphr;
EventGroupHandle_t mqtt_event_group;

static void status_callback(esp_mqtt_status_t status) {

    switch (status) {
        case ESP_MQTT_STATUS_CONNECTED:

            //Set connected bit within eventgroup
            if(mqtt_event_group != NULL)
                xEventGroupSetBits( mqtt_event_group, MQTT_CONNECTED_BIT );

            //Release semaphore and allow publishing
            xSemaphoreGive( mqttSemaphr );
                 
            //Format subscription topic by concat the topic strings
            char topic[256];
            sprintf( topic, "%s%s/#", pSubTopic, pClientId );
            
            // subscribe  
            if( !esp_mqtt_subscribe(topic, 2) ) {
                ESP_LOGE("MQTT", "Subscription failed");
            } else {
                ESP_LOGI("MQTT", "Subscriped to %s", topic);
            }

            break;

        case ESP_MQTT_STATUS_DISCONNECTED:

            //Clear connected bit in eventgoup
            if(mqtt_event_group != NULL)
                xEventGroupClearBits( mqtt_event_group, MQTT_CONNECTED_BIT );

            //Take semamphore in case we are not connected anymore
            xSemaphoreTake( mqttSemaphr, 100 );

            // reconnect
            esp_mqtt_start(pHost, pPort, pClientId, pUsername, pPassword);
            break;
    }

}

static void message_callback(const char *topic, uint8_t *payload, size_t len) {

    ESP_LOGI("MQTT", "incoming: %s => %s (%d)", topic, payload, (int)len);

#if CONFIG_LOG_DEFAULT_LEVEL >= 4 //Only enable this feature for debug log level
    /* Debug options: */
    if( strstr(topic, "/valve") ) {//Topic contains valve position
        uint valvePosition = 0;
        //Parse value from string
        sscanf( (char*) payload, "%u", &valvePosition );
        ESP_LOGI("MQTT", "Set valve position to %u%%", (uint8_t) valvePosition );
        //Set valve position
        if( valve_set( valvePosition ) != true )
            ESP_LOGE( "MQTT", "Valve set returned false. Regulation failed" );
    }
#endif

    /* Temperature target vlaue for this device */
    if( strstr(topic, "/temperature") ) {//Topic contains led at last elment
        
        float targetTemp = 0.0;
        
        //Parse temperature value from string
        sscanf( (char*) payload, "%f", &targetTemp );
        ESP_LOGI("MQTT", "Target temperature set to %2.1f°C", targetTemp );
        
        //Set target temperature
        setTemperature( targetTemp );
    }

}


void mqttClient_init(const char* host, const char* port, const char* username, const char* password, const char* pub_topic_prefix, const char* sub_topic_prefix) {

    pHost = host;
    pPort = port;
    pUsername = username;
    pPassword = password;
    
    //Store topic strings
    pPubTopic = pub_topic_prefix;
    pSubTopic = sub_topic_prefix;

    sprintf( pClientId, "%06llx", wlan_get_mac_lsb_first() );
    ESP_LOGI("MQTT", "Client id is: %s", pClientId);

    //Create mutex
    mqttSemaphr = xSemaphoreCreateMutex();

    //Create eventgroup
    if( mqtt_event_group == NULL )
        mqtt_event_group = xEventGroupCreate();

    //Take sempahore because mqtt connection is not established
    xSemaphoreTake( mqttSemaphr, 100 );

    //Init the MQTT client
    esp_mqtt_init(status_callback, message_callback, 256, 2000); //Was 1000ms

}

void mqttClient_task( void* pvParameters  ) {

    //Check for existing event group
    assert( wifi_event_group );

    //Task loop for stopping and starting mqtt client according to WiFi state
    while(1) {

        //Wait for connection established
        while( xEventGroupWaitBits( wifi_event_group, WIFI_CONNECTED_BIT, false, false, portMAX_DELAY ) != WIFI_CONNECTED_BIT )
            {} //Wait for wifi connection

        //Start mqtt client process
        esp_mqtt_start( pHost, pPort, pClientId, pUsername, pPassword );   
        
        //loop, as long as a wifi connection is established
        while( xEventGroupGetBits(wifi_event_group) & WIFI_CONNECTED_BIT ) {
            vTaskDelay( 1000/portTICK_RATE_MS );
        }

        //Stop mqtt process
        esp_mqtt_stop();
    }
}
    
void mqttClient_pubTemperature(float temperature) {

    //Publish only if semaphore is available
    if( xSemaphoreTake( mqttSemaphr, 10) == pdTRUE ) {
        char topic[256];
        char payload[10];
       
        //Format topic by concat the topic strings
        sprintf( topic, "%s%s/%s", pPubTopic, pClientId, TOPIC_TEMPERATURE );
        
        //Format payload from float to string
        sprintf( payload, "%2.1f", temperature );

        ESP_LOGI("MQTT", "Publish: \"%s\" to \"%s\"", payload, topic);
        
        //Send MQTT Message
        esp_mqtt_publish( topic, (uint8_t*) payload, strlen(payload), 0, false );

        //Release semaphore
        xSemaphoreGive( mqttSemaphr );
    }
}

void mqttClient_pubHumidity(float humidity) {

    //Publish only if semaphore is available
    if( xSemaphoreTake( mqttSemaphr, 10) == pdTRUE ) {
        char topic[256];
        char payload[10];
       
        //Format topic by concat the topic strings
        sprintf( topic, "%s%s/%s", pPubTopic, pClientId, TOPIC_HUMIDITY );
        
        //Format payload from float to string
        sprintf( payload, "%2.1f", humidity );

        ESP_LOGI("MQTT", "Publish: \"%s\" to \"%s\"", payload, topic);
        
        //Send MQTT Message
        esp_mqtt_publish( topic, (uint8_t*) payload, strlen(payload), 0, false );

        //Release semaphore
        xSemaphoreGive( mqttSemaphr );
    }
}

void mqttClient_pubValve(uint8_t percent) {

    //Publish only if semaphore is available
    if( xSemaphoreTake( mqttSemaphr, 10) == pdTRUE ) {
        char topic[256];
        char payload[10];
       
        //Format topic by concat the topic strings
        sprintf( topic, "%s%s/%s", pPubTopic, pClientId, TOPIC_VALVE );
        
        //Format payload from float to string
        sprintf( payload, "%u", percent );

        ESP_LOGI("MQTT", "Publish: \"%s\" to \"%s\"", payload, topic);
        
        //Send MQTT Message
        esp_mqtt_publish( topic, (uint8_t*) payload, strlen(payload), 0, false );

        //Release semaphore
        xSemaphoreGive( mqttSemaphr );
    }
}

void mqttClient_pubBattery(float voltage) {
    
    //Publish only if semaphore is available
    if( xSemaphoreTake( mqttSemaphr, 10) == pdTRUE ) {
        char topic[256];
        char payload[10];
       
        //Format topic by concat the topic strings
        sprintf( topic, "%s%s/%s", pPubTopic, pClientId, TOPIC_BATTERY );
        
        //Format payload from float to string
        sprintf( payload, "%2.1f", voltage );

        ESP_LOGI("MQTT", "Publish: \"%s\" to \"%s\"", payload, topic);
        
        //Send MQTT Message
        esp_mqtt_publish( topic, (uint8_t*) payload, strlen(payload), 0, false );

        //Release semaphore
        xSemaphoreGive( mqttSemaphr );
    }

}