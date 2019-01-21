/*

DEBUG MODULE

*/

#include <Arduino.h>
#include "debug.h"
#include "configuration.h"

void debugSetup() {

    #if defined(DEBUG_PORT)
        DEBUG_PORT.begin(DEBUG_PORT_BAUDRATE);
        while (!DEBUG_PORT && millis() < 5000);
        DEBUG_PORT.println();
        DEBUG_PORT.println();
    #endif

}
