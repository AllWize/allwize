/*

Allwize - Simple MASTER example

Listents to messages on the same channel and CF and
prints them out via the serial monitor

Copyright (C) 2018 by Allwize <github@allwize.io>

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

// -----------------------------------------------------------------------------
// Board definitions
// -----------------------------------------------------------------------------

#if defined(ARDUINO_AVR_UNO)
    #define RX_PIN      8
    #define TX_PIN      9
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
    #define debug       Serial
#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)
    #define module      Serial1
    #define debug       Serial
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define module      Serial1
    #define debug       SerialUSB
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)
    #define RX_PIN      12
    #define TX_PIN      13
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
    #define debug       Serial
#endif // ARDUINO_ARCH_ESP8266

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL        0x04
#define WIZE_DATARATE       0x01
#define WIZE_NETWORK_ID     0x46

// -----------------------------------------------------------------------------
// Wize
// -----------------------------------------------------------------------------

#include "Allwize.h"
Allwize * allwize;

void wizeSetup() {

    module.begin(19200);
    #if USE_SNIFFER
        sniffer = new SerialSniffer(module, debug);
        allwize = new Allwize(*sniffer);
    #else
        allwize = new Allwize(module);
    #endif
    allwize->begin();
    while (!allwize->ready());

    allwize->master();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(5);
    allwize->setDataRate(WIZE_DATARATE);
    allwize->setControlField(WIZE_NETWORK_ID);

    allwize->dump(debug);

    debug.println("[WIZE] Listening...");

}

void wizeLoop() {

    if (allwize->available()) {

        allwize_message_t message = allwize->read();

        // Code to pretty-print the message
        char buffer[64];
        char ascii[message.len+1];

        snprintf(buffer, sizeof(buffer), "[WIZE] C: 0x%02X, CI: 0x%02X, RSSI: 0x%02X, DATA: { ", message.c, message.ci, message.rssi);
        debug.print(buffer);

        for (uint8_t i=0; i<message.len; i++) {
            char ch = message.data[i];
            snprintf(buffer, sizeof(buffer), "0x%02X ", ch);
            debug.print(buffer);
            ascii[i] = (31 < ch && ch < 127) ? ch : ' ';
        }
        ascii[message.len] = 0;
        debug.print("}, STR: \"");
        debug.print(ascii);
        debug.println("\"");

    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Setup serial debug
    debug.begin(115200);
    while (!debug && millis() < 5000);
    debug.println();
    debug.println("[MAIN] Wize Master Example");
    debug.println();

    // Init radio
    wizeSetup();

}

void loop() {

    // Listen to messages
    wizeLoop();

}
