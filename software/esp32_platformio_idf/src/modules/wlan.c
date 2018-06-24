#include "wlan.h"
#include "board/config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

static const char *TAG = "WLAN";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;

    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d", MAC2STR(event->event_info.sta_connected.mac), event->event_info.sta_connected.aid);
        break;

    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d", MAC2STR(event->event_info.sta_disconnected.mac), event->event_info.sta_disconnected.aid);
        break;
        
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;

    default:
        break;
    }
    return ESP_OK;
}

void wlan_init() {
    
    wifi_init_config_t init_conf = WIFI_INIT_CONFIG_DEFAULT();

    //Init tcpip stack
    tcpip_adapter_init();

    //set hostname
    tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, "ESP32-Radiator");

    //init event loop
    esp_event_loop_init(event_handler, NULL);

    //Init wifi peripheral -> Must be done first
    esp_wifi_init( &init_conf );

    //Ram storage
    esp_wifi_set_storage(WIFI_STORAGE_RAM);

    //Set wifi into station mode
    esp_wifi_set_mode( WIFI_MODE_STA );

    //Set country to europe
    esp_wifi_set_country( WIFI_COUNTRY_POLICY_AUTO );
    
    wifi_config_t wifi_conf = {
        .sta =  {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        }
    };

    esp_wifi_set_config( ESP_IF_WIFI_STA, &wifi_conf );

    esp_wifi_set_ps( WIFI_PS_MODEM );

    //Start wifi module
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             WIFI_SSID, WIFI_PASSWORD);
}

uint64_t wlan_get_mac_lsb_first() {
    uint8_t mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);

    //Swap bytes to get 0xaabbccddeeff for mac AA:BB:CC:DD:EE:FF
    uint8_t mac_swp[8] = { mac[5], mac[4], mac[3], mac[2], mac[1], mac[0], 00, 00 };

    return *( (uint64_t*) mac_swp );
}

void wlan_sleep() {

    esp_wifi_stop();

}