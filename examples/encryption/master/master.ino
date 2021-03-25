/*

AllWize - Simple Master example with Encryption

Listens to messages on the same channel, data rate.
Decrypts the message using module built-in features and
prints them out via the serial monitor.

Copyright (C) 2018 by AllWize <github@allwize->io>

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
    #define RESET_PIN               14
    #define RX_PIN                  5
    #define TX_PIN                  4
    #define DEBUG_SERIAL            Serial
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

#define SLAVE_MID               0x06FA          // AWZ
#define SLAVE_UID               0x01020304      // Test device
#define SLAVE_VERSION           0x01
#define SLAVE_TYPE              0x01
#define SLAVE_REGISTER          0x01
uint8_t KMAC_KEY[16] = { 0x71, 0x5A, 0xD8, 0x83, 0x5B, 0xC9, 0x54, 0x70, 0x26, 0x0B, 0xA3, 0x40, 0x92, 0xA8, 0x73, 0x98 };
uint8_t KENC_KEY[16] = { 0x4F, 0x0E, 0x26, 0x90, 0x2B, 0x0E, 0x2A, 0x54, 0x77, 0xB9, 0xA6, 0x30, 0x93, 0x54, 0xC1, 0x62 };

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
    allwize->setDataRate(WIZE_DATARATE);

    // Register slave
    allwize->setKmac(KMAC_KEY);
    allwize->setKenc(SLAVE_REGISTER, KENC_KEY);
    allwize->bindSlave(SLAVE_REGISTER, SLAVE_MID, SLAVE_UID, SLAVE_VERSION, SLAVE_TYPE);
    allwize->setMAC2CheckOnlyFlag(1);

    delay(1000);

    allwize->binds(DEBUG_SERIAL);
    allwize->dump(DEBUG_SERIAL);

    char buffer[64];
    if (MODULE_WIZE == allwize->getModuleType()) {
        snprintf(buffer, sizeof(buffer), "[WIZE] Channel   : %d (WIZE_%d)\n", allwize->getChannel(), allwize->getChannel() * 10 + 90); DEBUG_SERIAL.print(buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "[WIZE] Channel   : %d\n", allwize->getChannel()); DEBUG_SERIAL.print(buffer);
    }
    snprintf(buffer, sizeof(buffer), "[WIZE] Frequency : %.6f MHz\n", allwize->getFrequency(allwize->getChannel())); DEBUG_SERIAL.print(buffer);
    snprintf(buffer, sizeof(buffer), "[WIZE] Datarate  : %d bps\n", allwize->getDataRateSpeed(allwize->getDataRate())); DEBUG_SERIAL.print(buffer);
    DEBUG_SERIAL.print("[WIZE] Listening...\n");

}


void wizeLoop() {

    if (allwize->available()) {

        // Get the RAW message
        uint8_t * message = allwize->getBuffer();
        uint8_t length = allwize->getLength();

        // Print it
        char buffer[6];
        DEBUG_SERIAL.print("[WIZE] Received: ");
        for (uint8_t i=1; i<length-1; i++) { // first & last bytes are start & stop bytes, no need to print them
            snprintf(buffer, sizeof(buffer), "%02X", message[i]);
            DEBUG_SERIAL.print(buffer);
        }
        DEBUG_SERIAL.println();

        //allwize->sendChallenge(KENC_KEY);
        //allwize->sendNoResponse();

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
    DEBUG_SERIAL.println("[MAIN] Wize Master Encrypted Example");
    DEBUG_SERIAL.println();

    // Init radio
    wizeSetup();

}

void loop() {

    // Listen to messages
    wizeLoop();

}
