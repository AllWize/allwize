/*

AllWize - Simple Slave example with Encryption

Sends messages forcing the module to encrypt them.

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

#if defined(ARDUINO_ALLWIZE_K2)
    #define HARDWARE_SERIAL     SerialWize
    #define DEBUG_SERIAL        SerialUSB
    #define RESET_PIN           PIN_WIZE_RESET
#elif defined(ARDUINO_ARCH_SAMD)
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

#define SLAVE_MID               0x06FA          // AWZ
#define SLAVE_UID               0x01020304      // Test device
#define SLAVE_VERSION           0x01
#define SLAVE_TYPE              0x01
#define SLAVE_REGISTER          0x01
uint8_t KMAC_KEY[16] = { 0x71, 0x5A, 0xD8, 0x83, 0x5B, 0xC9, 0x54, 0x70, 0x26, 0x0B, 0xA3, 0x40, 0x92, 0xA8, 0x73, 0x98 };
uint8_t KENC_KEY[16] = { 0x4F, 0x0E, 0x26, 0x90, 0x2B, 0x0E, 0x2A, 0x54, 0x77, 0xB9, 0xA6, 0x30, 0x93, 0x54, 0xC1, 0x62 };

// -----------------------------------------------------------------------------
// AllWize
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

    allwize->slave();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);
    
    // Set the device identifiers
    allwize->setMID(SLAVE_MID);
    allwize->setUID(SLAVE_UID);
    allwize->setDeviceVersion(SLAVE_VERSION);
    allwize->setDeviceType(SLAVE_TYPE);
    allwize->setKmac(KMAC_KEY);
    allwize->setKenc(SLAVE_REGISTER, KENC_KEY);
    allwize->selectKenc(SLAVE_REGISTER);

    allwize->dump(DEBUG_SERIAL);

    char buffer[64];
    if (MODULE_WIZE == allwize->getModuleType()) {
        snprintf(buffer, sizeof(buffer), "[WIZE] Channel   : %d (WIZE_%d)\n", allwize->getChannel(), allwize->getChannel() * 10 + 90); DEBUG_SERIAL.print(buffer);
    } else {
        snprintf(buffer, sizeof(buffer), "[WIZE] Channel   : %d\n", allwize->getChannel()); DEBUG_SERIAL.print(buffer);
    }
    snprintf(buffer, sizeof(buffer), "[WIZE] Frequency : %.6f MHz\n", allwize->getFrequency(allwize->getChannel())); DEBUG_SERIAL.print(buffer);
    snprintf(buffer, sizeof(buffer), "[WIZE] Datarate  : %d bps\n", allwize->getDataRateSpeed(allwize->getDataRate())); DEBUG_SERIAL.print(buffer);
    DEBUG_SERIAL.println("[WIZE] Ready...");

}

void wizeSend(uint8_t * payload, size_t len) {

    char buffer[64];
    DEBUG_SERIAL.print("[WIZE] Sending: ");
    for (uint8_t i = 0; i<len; i++) {
        snprintf(buffer, sizeof(buffer), "%02X", payload[i]);
        DEBUG_SERIAL.print(buffer);
    }
    DEBUG_SERIAL.print("\n");

    if (!allwize->send(payload, len)) {
        DEBUG_SERIAL.println("[WIZE] Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Init serial DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[MAIN] Basic slave example");

    // Init radio
    wizeSetup();

}

void loop() {

    // counter
    static uint8_t count = 0;

    // Build payload
    uint8_t payload[6];
    for (uint8_t i=0; i<sizeof(payload); i++) payload[i] = (i + count) & 0xFF;
    wizeSend(payload, sizeof(payload));
    count++;

    // Send the string as payload
    wizeSend(payload, sizeof(payload));

    // Wait 10 seconds and redo
    delay(10000);

}
