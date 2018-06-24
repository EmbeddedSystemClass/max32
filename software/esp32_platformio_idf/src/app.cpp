#include "app.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "board/config.h"
#include "board/board.h"
#include "board/interfaces.h"
#include "driver/drv883x.h"
#include "driver/si7020.h"
#include "modules/wlan.h"
#include "modules/valve.h"
#include "services/mdnsService.h"
#include "tasks/mqttClient.h"
#include "tasks/heatCtrl.h"

//Global variables
EventGroupHandle_t wifi_event_group = NULL;

 int app() {
     
    ESP_LOGD( "SYS", "Starting up..." );

    /* Chip function init */
    ESP_LOGD( "SYS", "Chip init" );
    //Non-Volatile memory
    nvs_flash_init();

    /* Init all hardware modules which are board specific e.g. GPIO */
    ESP_LOGD( "SYS", "Board/Hardware init" );

    board_init();

    /* Temperature and Humidity Sensor Test*/
    ESP_LOGD( "SYS", "I2C Init" );

    //ESP32 I²C module init
    i2c_init();

    ESP_LOGD("SYS", "Pre-Task init" );

    /* WiFi */
    ESP_LOGD("SYS", "WiFi init" );

    //Create event group for wifi state
    wifi_event_group = xEventGroupCreate();

    if( wifi_event_group == NULL ) {
        ESP_LOGE("SYS", "Wifi event group creation failed" );
    }

    wlan_init();
    
    /* mDNS Service */
    ESP_LOGD( "SYS", "mDNS init" );
    
    MDNSService mdnsService;
    mdnsService.start();

    /* Heat controller */
    ESP_LOGD( "SYS", "Heat controller creation" );

    TaskHandle_t heatController = NULL;
    xTaskCreatePinnedToCore( heatController_task, "heatCtrl", 4096, xTaskGetCurrentTaskHandle(), tskIDLE_PRIORITY+1, &heatController, 0);

    /* MQTT Client */
    ESP_LOGD( "SYS", "MQTT Client init" );

    mqttClient_init( MQTT_BROKER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD, MQTT_PUBLICATION_PREFIX, MQTT_SUBSCRIPTION_PREFIX );
    xTaskCreate( mqttClient_task, "MQTT", 4096, NULL, tskIDLE_PRIORITY+10, NULL );
    
    ESP_LOGD("SYS", "System startup done");
    
    uint8_t a = 0; //Toggle variable
    while(1) {

        if(a++ >= 2)
            a = 0;

        board_setLed( a );

        //Wait for end of heat controlling and the sleep
        if( ulTaskNotifyTake( pdTRUE, DELAY_MS(1000) ) > 0 ) {

            //Set wlan to sleep
            wlan_sleep();

            //keep RTC RAM powered during deep sleep
            esp_sleep_pd_config( ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON );
        
            ESP_LOGD( "SYS", "Enter deep sleep");

            //Deep sleep for 50 Seconds (50 * 10^6 µs)
            esp_deep_sleep( 50 * 1E6 );
        }

    }
 }