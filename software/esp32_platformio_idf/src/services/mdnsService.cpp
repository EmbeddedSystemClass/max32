#include "mdnsService.h"
#include "mdns.h"

MDNSService::MDNSService() {

    _hostname = "MAX!ESP32";
    _instancename ="MAX!-001";

}

void MDNSService::start() {

    //initialize mDNS service
    esp_err_t err = mdns_init();

    if (err) {
        printf("MDNS Init failed: %d\n", err);
        return;
    }

    //set hostname
    mdns_hostname_set(_hostname);
    
    //set default instance
    mdns_instance_name_set(_instancename);
}