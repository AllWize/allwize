/*

AllWize - Simple Master example

Listens to messages on the same channel, data rate and CF and
prints them out via the serial monitor.

Copyright (C) 2018 by AllWize <github@allwize.io>

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
    #define RESET_PIN           7
    #define RX_PIN              8
    #define TX_PIN              9
    #define DEBUG_SERIAL        Serial
#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)
    #define RESET_PIN           7
    #define HARDWARE_SERIAL     Serial1
    #define DEBUG_SERIAL        Serial
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define RESET_PIN           7
    #define HARDWARE_SERIAL     Serial1
    #define DEBUG_SERIAL        SerialUSB
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)
    #define RESET_PIN           14
    #define RX_PIN              12
    #define TX_PIN              13
    #define DEBUG_SERIAL        Serial
#endif // ARDUINO_ARCH_ESP8266

#if defined(ARDUINO_ARCH_ESP32)
    #define RESET_PIN           14
    #define RX_PIN              12
    #define TX_PIN              13
    #define DEBUG_SERIAL        Serial
#endif // ARDUINO_ARCH_ESP32

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps

#define WIZE_ENCRYPT_KEY        { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c }

// -----------------------------------------------------------------------------
// Wize
// -----------------------------------------------------------------------------

#include "AllWize.h"
AllWize * allwize;

void wizeSetup() {

    // Create and init AllWize object
    #if defined(HARDWARE_SERIAL)
        allwize = new AllWize(&HARDWARE_SERIAL, RESET_PIN);
    #else
        allwize = new AllWize(RX_PIN, TX_PIN, RESET_PIN);
    #endif
    allwize->begin();
    if (!allwize->waitForReady()) {
        DEBUG_SERIAL.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize->master();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);

    //allwize->dump(DEBUG_SERIAL);

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "[WIZE] Listening... CH %d, DR %d\n", allwize->getChannel(), allwize->getDataRate());
    DEBUG_SERIAL.print(buffer);

}

void wizeDebugMessage(allwize_message_t message) {

    // Code to pretty-print the message
    char buffer[128];
    snprintf(
        buffer, sizeof(buffer),
        "[WIZE] C: 0x%02X, MAN: %s, ADDR: 0x%02X%02X%02X%02X, TYPE: 0x%02X, VERSION: 0x%02X, CI: 0x%02X, RSSI: %d, DATA: { ",
        message.c,
        message.man,
        message.address[0], message.address[1],
        message.address[2], message.address[3],
        message.type, message.version,
        message.ci, (int16_t) message.rssi / -2
    );
    DEBUG_SERIAL.print(buffer);

    for (uint8_t i=0; i<message.len; i++) {
        char ch = message.data[i];
        snprintf(buffer, sizeof(buffer), "0x%02X ", ch);
        DEBUG_SERIAL.print(buffer);
    }
    DEBUG_SERIAL.print("}, STR: \"");
    DEBUG_SERIAL.print((char *) message.data);
    DEBUG_SERIAL.println("\"");

}

void wizeLoop() {

    if (allwize->available()) {

        // Get the message
        allwize_message_t message = allwize->read();

        // Decrypt
        #if defined(ALLWIZE_EXTERNAL_AES) && defined(WIZE_ENCRYPT_KEY)
            uint8_t key[] = WIZE_ENCRYPT_KEY;
            allwize->decrypt(message.data, key, message.len);
        #endif
        
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
