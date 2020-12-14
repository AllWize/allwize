/*

AllWize - Simple Slave Example - AllWize K2 w/ TPL5111

Simple slave that sends an auto-increment number every 5 seconds.

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
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps
#define WIZE_UID                0x20212223

#define LED_PIN                 6
#define DONE_PIN                5

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

AllWize allwize(&MODULE_SERIAL, RESET_PIN);

// -----------------------------------------------------------------------------
// TPL5110
// -----------------------------------------------------------------------------

void powerSetup() {
    pinMode(DONE_PIN, OUTPUT);
    digitalWrite(DONE_PIN, LOW);
}

void powerOff() {
    delay(100);
    digitalWrite(DONE_PIN, HIGH);
    delay(100);
    digitalWrite(DONE_PIN, LOW);
    delay(100);
}

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

void wizeSetup() {

    DEBUG_SERIAL.println("Initializing radio module");

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize.slave();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);
    allwize.setUID(WIZE_UID);

    allwize.dump(DEBUG_SERIAL);

    DEBUG_SERIAL.println("[WIZE] Ready...");

}

void wizeSend(const char * payload) {

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "[WIZE] Sending '%s'\n", payload);
    DEBUG_SERIAL.print(buffer);

    if (!allwize.send(payload)) {
        DEBUG_SERIAL.println("[WIZE] Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    // Init serial DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[MAIN] Basic slave example");
   
    // TPL5110 interface
    powerSetup();

    // Init radio
    wizeSetup();
    char payload[4] = "TPL";
    wizeSend(payload);

    delay(1000);
    powerOff();

}

void loop() {
}
