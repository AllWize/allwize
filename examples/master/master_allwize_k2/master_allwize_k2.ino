/*

AllWize K2

Listens to messages on the same channel and data rate
and prints them out via the serial monitor.

Copyright (C) 2018-2021 by AllWize <github@allwize.io>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "AllWize.h"

// -----------------------------------------------------------------------------
// Board configuration
// -----------------------------------------------------------------------------

// Check http://wiki.allwize.io/index.php?title=Allwize_K2#Arduino_IDE
// to add support to Allwize K2 board in Arduino IDE
#if not defined(ARDUINO_ALLWIZE_K2)
    #error "This example is meant to run on an AllWize K2 board!"
#endif

#define MODULE_SERIAL           SerialWize
#define DEBUG_SERIAL            SerialUSB
#define RESET_PIN               PIN_WIZE_RESET

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_DATARATE           DATARATE_2400bps

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

AllWize allwize(&MODULE_SERIAL, RESET_PIN);

// -----------------------------------------------------------------------------
// Wize
// -----------------------------------------------------------------------------

void wizeSetup() {

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize.master();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setDataRate(WIZE_DATARATE);

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "[WIZE] Listening... CH %d, DR %d\n", allwize.getChannel(), allwize.getDataRate());
    DEBUG_SERIAL.print(buffer);

}

void wizeDebugMessage(allwize_message_t message) {

    // Code to pretty-print the message
    char buffer[128];
    if (CI_WIZE == message.ci) {
        snprintf(
            buffer, sizeof(buffer),
            "[WIZE] C: 0x%02X, CI: 0x%02X, MAN: %s, ADDR: 0x%02X%02X%02X%02X, CONTROL: %d, OPID: %d, APPID: %d, COUNTER: %d, RSSI: %d, DATA: 0x",
            message.c, message.ci, 
            message.man,
            message.address[0], message.address[1],
            message.address[2], message.address[3],
            message.wize_control, message.wize_network_id, message.wize_application, message.wize_counter,
            (int16_t) message.rssi / -2
        );
    } else {
        snprintf(
            buffer, sizeof(buffer),
            "[WIZE] C: 0x%02X, CI: 0x%02X, MAN: %s, ADDR: 0x%02X%02X%02X%02X, RSSI: %d, DATA: 0x",
            message.c, message.ci, 
            message.man,
            message.address[0], message.address[1],
            message.address[2], message.address[3],
            (int16_t) message.rssi / -2
        );
    }
    DEBUG_SERIAL.print(buffer);

    for (uint8_t i=0; i<message.len; i++) {
        snprintf(buffer, sizeof(buffer), "%02X", message.data[i]);
        DEBUG_SERIAL.print(buffer);
    }
    DEBUG_SERIAL.println();

}

void wizeLoop() {

    if (allwize.available()) {

        // Get the message
        allwize_message_t message = allwize.read();

        // Show it to console
        wizeDebugMessage(message);

    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Setup serial DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[MAIN] Wize Master Example");
    DEBUG_SERIAL.println();

    // Init radio
    wizeSetup();

}

void loop() {

    // Listen to messages
    wizeLoop();

}
