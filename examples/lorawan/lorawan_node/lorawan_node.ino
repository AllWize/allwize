/*

AllWize - LoRaWAN Node example

Simple slave that sends an auto-increment number every 5 seconds.

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
#define WIZE_NODE_ID            0x10

#define LORAWAN_PORT            1

uint8_t DEVADDR[4] = { 0x26, 0x01, 0x11, 0xF4 };
uint8_t NWKSKEY[16] = { 0x52, 0x08, 0x08, 0x50, 0xA8, 0xC0, 0xD9, 0x9A, 0xD1, 0x99, 0x53, 0xF0, 0x5F, 0xBC, 0xF6, 0xC1 };
uint8_t APPSKEY[16] = { 0x4F, 0x0E, 0x26, 0x90, 0x2B, 0x0E, 0x2A, 0x54, 0x77, 0xB9, 0xA6, 0x30, 0x93, 0x54, 0xC1, 0x62 };

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

#include "AllWize_LoRaWAN.h"
AllWize_LoRaWAN * allwize;

void wizeSetup() {

    // Create and init AllWize object
    #if defined(HARDWARE_SERIAL)
        allwize = new AllWize_LoRaWAN(&HARDWARE_SERIAL, RESET_PIN);
    #else
        allwize = new AllWize_LoRaWAN(RX_PIN, TX_PIN, RESET_PIN);
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
    allwize->setControlInformation(WIZE_NODE_ID);

    // LoRaWan settings
    allwize->joinABP(DEVADDR, APPSKEY, NWKSKEY);

    DEBUG_SERIAL.println("[WIZE] Ready...");

}

void wizeSend(uint8_t * payload, size_t len) {

    char buffer[64];
    snprintf(buffer, sizeof(buffer),
        "[WIZE] CH: %d, TX: %d, DR: %d, Payload: ",
        allwize->getChannel(), allwize->getPower(), allwize->getDataRate()
    );
    DEBUG_SERIAL.print(buffer);

    for (uint8_t i = 0; i<len; i++) {
        snprintf(buffer, sizeof(buffer), "%02X", payload[i]);
        DEBUG_SERIAL.print(buffer);
    }
    DEBUG_SERIAL.print("\n");

    if (!allwize->send(payload, len, LORAWAN_PORT)) {
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
    DEBUG_SERIAL.println("[MAIN] LoRaWAN node");

    // Init radio
    wizeSetup();

}

void loop() {

    // This static variables will hold the number as int and char string
    static uint8_t count = 0;

    // Payload
    uint8_t payload[1] = { count };

    // Send the string as payload
    wizeSend(payload, 1);

    // Increment the number (it will overflow at 255)
    count++;

    // Wait 20 seconds and redo
    delay(20000);

}
