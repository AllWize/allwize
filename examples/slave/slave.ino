/*

Allwize - Simple Slave Example

Simple slave that sends an auto-increment number every 5 seconds.

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
    #define RESET_PIN   7
    #define RX_PIN      8
    #define TX_PIN      9
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
    #define debug       Serial
#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)
    #define RESET_PIN   7
    #define module      Serial1
    #define debug       Serial
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define RESET_PIN   7
    #define module      Serial1
    #define debug       SerialUSB
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)
    #define RESET_PIN   14
    #define RX_PIN      12
    #define TX_PIN      13
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
    #define debug       Serial
#endif // ARDUINO_ARCH_ESP8266


// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps
#define WIZE_NETWORK_ID         0x46
#define WIZE_NODE_ID            0x02

// -----------------------------------------------------------------------------
// Allwize
// -----------------------------------------------------------------------------

#include "Allwize.h"
Allwize * allwize;

void wizeSetup() {

    allwize = new Allwize(module, RESET_PIN);
    allwize->reset();
    module.begin(19200);
    while (!allwize->ready());
    allwize->begin();

    allwize->slave();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);
    allwize->setControlField(WIZE_NETWORK_ID);
    allwize->setControlInformation(WIZE_NODE_ID);

}

void wizeSend(const char * payload) {

    debug.print("[Allwize] Payload: ");
    debug.println(payload);

    if (!allwize->send(payload)) {
        debug.println("[Allwize] Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Init serial debug
    debug.begin(115200);
    while (!debug && millis() < 5000);
    debug.println();
    debug.println("[Allwize] MCP9701 sensor example");

    // Init radio
    wizeSetup();

}

void loop() {

    // This static variable will hold the number
    static uint8_t count = 0;

    // Convert the number to a string
    char payload[4];
    itoa(count, payload, 10);

    // Send the string as payload
    wizeSend(payload);

    // Increment the number (it will overflow at 255)
    count++;

    // Wait 5 seconds and redo
    delay(5000);

}
