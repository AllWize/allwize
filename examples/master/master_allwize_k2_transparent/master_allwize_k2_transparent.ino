/*

AllWize K2

Listens to messages on the same channel and data rate
and prints them RAW out via the serial monitor.

Copyright (C) 2018-2020 by AllWize <github@allwize.io>

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
    allwize.setMode(MBUS_MODE_N2);
    allwize.setChannel(WIZE_CHANNEL);
    allwize.setDataRate(WIZE_DATARATE);
    allwize.setMAC2CheckOnlyFlag(1);

    char buffer[64];
    if (MODULE_WIZE == allwize.getModuleType()) {
        snprintf(buffer, sizeof(buffer), "[WIZE] Channel   : %d (WIZE_%d)\n", allwize.getChannel(), allwize.getChannel() * 10 + 90); DEBUG_SERIAL.print(buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "[WIZE] Channel   : %d\n", allwize.getChannel()); DEBUG_SERIAL.print(buffer);
    }
    snprintf(buffer, sizeof(buffer), "[WIZE] Frequency : %.6f MHz\n", allwize.getFrequency(allwize.getChannel())); DEBUG_SERIAL.print(buffer);
    snprintf(buffer, sizeof(buffer), "[WIZE] Datarate  : %d bps\n", allwize.getDataRateSpeed(allwize.getDataRate())); DEBUG_SERIAL.print(buffer);
    DEBUG_SERIAL.print("[WIZE] Listening...\n");

}

void wizeLoop() {

    if (allwize.available()) {

        // Get the RAW message
        uint8_t * message = allwize.getBuffer();
        uint8_t length = allwize.getLength();

        // Print it
        char buffer[6];
        DEBUG_SERIAL.print("[WIZE] Received: ");
        for (uint8_t i=1; i<length-1; i++) { // first & last bytes are start & stop bytes, no need to print them
            snprintf(buffer, sizeof(buffer), "%02X", message[i]);
            DEBUG_SERIAL.print(buffer);
        }
        DEBUG_SERIAL.println();

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
    DEBUG_SERIAL.println("[MAIN] Wize Master Example - RAW output");
    DEBUG_SERIAL.println();

    // Init radio
    wizeSetup();

}

void loop() {

    // Listen to messages
    wizeLoop();

}
