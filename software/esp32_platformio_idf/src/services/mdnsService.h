#ifndef MDNSSERVICE
#define MDNSSERVICE

#include "mdns.h"

class MDNSService {

private:
    const char* _hostname;
    const char* _instancename;

public:
    MDNSService();
    void start();    
};

#endif //MDNSSERVICE