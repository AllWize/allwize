/*

AllWize - Parking Use Case
Using an ultrasonic HC-SR04

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

#if defined(ARDUINO_AVR_LEONARDO)
    #define RESET_PIN           7
    #define MODULE_SERIAL       Serial1
    #define DEBUG_SERIAL        Serial
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define RESET_PIN           7
    #define DEBUG_SERIAL        SerialUSB
    #define MODULE_SERIAL       Serial1
#endif // ARDUINO_ARCH_SAMD

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL        CHANNEL_04
#define WIZE_POWER          POWER_20dBm
#define WIZE_DATARATE       DATARATE_2400bps
#define WIZE_UID            0x20212223

#define TRIGGER_PIN         5
#define ECHO_PIN            6
#define THRESHOLD_DISTANCE  200     // in mm
#define SLEEP_TIME          5000    // in ms

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

#include "AllWize.h"
AllWize * allwize;

void wizeSetup() {

    DEBUG_SERIAL.println("Checking radio module");

    #if defined(ARDUINO_ARCH_SAMD) && defined(RX_PIN) && defined(TX_PIN)
        pinPeripheral(RX_PIN, SERCOM_MODE);
        pinPeripheral(TX_PIN, SERCOM_MODE);
    #endif

    // Create and init AllWize object
    allwize = new AllWize(&MODULE_SERIAL, RESET_PIN);
    allwize->begin();
    if (!allwize->waitForReady()) {
        DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize->slave();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);
    allwize->setUID(WIZE_UID);

    DEBUG_SERIAL.println("Radio module OK");

}

void wizeSend(const char * payload) {

    DEBUG_SERIAL.print("Payload: ");
    DEBUG_SERIAL.println(payload);

    if (!allwize->send(payload)) {
        DEBUG_SERIAL.println("Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Sensor
// -----------------------------------------------------------------------------

uint16_t getDistance() {

    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);

    uint32_t duration = pulseIn(ECHO_PIN, HIGH);
    uint16_t distance = (duration / 2) * 0.34; // in millimeters

    distance = constrain(distance, 0, 2000);
    return distance;

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void sleep(uint32_t ms) {
    // TODO: sleep radio and MCU
    delay(ms);
}

void setup() {

    // Init DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("Parking Use Case");

    // Init HC-SR04
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Init radio
    wizeSetup();

}

void loop() {

    uint16_t distance = getDistance();

    char payload[16];
    snprintf(payload, sizeof(payload), "%d,%d", distance, distance < THRESHOLD_DISTANCE ? 1 : 0);
    wizeSend(payload);

    sleep(SLEEP_TIME);

}
