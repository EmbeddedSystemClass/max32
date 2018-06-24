#include "heatCtrl.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "board/board.h"
#include "driver/si7020.h"
#include "modules/valve.h"
#include "tasks/mqttClient.h"

static QueueHandle_t heatTempQueue = NULL;

void heatController_task( void* pvParameters ) {

    //pvParameters must not be NULL
    assert( pvParameters );

    //pvParameters contain the TaskHandle for the parent task. This handle is used as callback mechanism
    TaskHandle_t parentTask = pvParameters;

    //Create a queue for setting the target temperature 
    heatTempQueue = xQueueCreate( 1, sizeof(float) );

    //Create instance of SI7020 sensor which measures Temperature and Humidity
    SI7020 sensor( SI7020_ADDR );
    uint8_t serial = sensor.getSerial();

    //Check connection and correrct sensor id
    if( serial != SI7020::SERIAL_NUMBER) {
        ESP_LOGE( "HEATC", "FAILED: Serial is %d and should %d", serial, SI7020::SERIAL_NUMBER );
    } else {
        ESP_LOGD( "HEATC", "Temperature: %2.2f\n", sensor.getTemperature() );
    }

    //Init valve
    ESP_LOGD( "HEATC", "Valve init\n" );
    
    valve_init();

    //Calibrate valve
    if( valve_calibration() != true ) {
        ESP_LOGE("HEATC", "Valve calibration failed");
    }

    float temperature;
    float humidity;

    //Regulate temperature
    while( 1 ) {
        
        temperature = sensor.getTemperature();
        humidity = sensor.getHumidity();

        //Wait for target Temperature but max. 10 seconds
        float targetTemp = 0;
        if( xQueueReceive( heatTempQueue, &targetTemp, DELAY_MS(10000) ) == pdTRUE ) {
            
            ESP_LOGD( "HEATC", "Target temperature set to %2.1f", targetTemp );

            mqttClient_pubTemperature( temperature );
            mqttClient_pubHumidity( humidity );

            //Regulate
            if( temperature >= targetTemp ) {
                valve_set(0); //Close valve on correct or over-temperature
            } else {
                //Open valve according to temperature difference - experimental
                uint16_t valveValue = (targetTemp - temperature) * 20; //On >=5°C difference, the valve will be opend fully
                valve_set( valveValue > 100 ? 100 : (uint8_t) valveValue ); //Make sure, the valve value will not be greater than 100%
            }
        }
        //vTaskDelay( DELAY_MS(10000) );

        //Exit loop
        break;
    }
    
    xTaskNotifyGive( parentTask );

    //Delete this task if all work is done and we can sleep
    vTaskDelete( NULL );

}

bool setTemperature( float temperature ) {

    assert( heatTempQueue );

    //Send new temperature value to task and return true on success
    return xQueueSend( heatTempQueue, &temperature, 100 );
}