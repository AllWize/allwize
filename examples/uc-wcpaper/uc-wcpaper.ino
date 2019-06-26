/*

AllWize - Shit happens

Copyright (C) 2018-2019 by AllWize <github@allwize.io>

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
    #define ANALOG_DEPTH        10
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define RESET_PIN           7
    #define DEBUG_SERIAL        SerialUSB
    #define MODULE_SERIAL       Serial1
    #define ANALOG_DEPTH        10
#endif // ARDUINO_ARCH_SAMD

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL        CHANNEL_04
#define WIZE_POWER          POWER_20dBm
#define WIZE_DATARATE       DATARATE_2400bps
#define WIZE_UID            0x20212223

#define POT_PIN             A0
#define SLEEP_TIME          5000    // in ms
#define SAMPLE_COUNT        10
#define ANALOG_MAX          (1 << ANALOG_DEPTH)

#define VALUE_FOR_0_DEG     526.0   // value of the pot when the lever is completely horizontal
#define VALUE_FOR_90_DEG    175.0   // value of the pot when the lever is completely vertical
#define ANGLE_THRESHOLD     22      // angle at which the lever lies flat over an empty toilet roll

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
// Sensors
// -----------------------------------------------------------------------------

int16_t getAngle() {

    double value = 0;
    for (unsigned char i=0; i<SAMPLE_COUNT; i++) {
        value += analogRead(POT_PIN);
    }
    value = value / SAMPLE_COUNT;

    // Uncomment this line to get a reading of the sensor immersed in water,
    // set HIGROMETER_MIN to this value
    //Serial.print("Sensor reading: "); Serial.println(value);

    value = map(value, VALUE_FOR_0_DEG, VALUE_FOR_90_DEG, 0.0, 90.0);
    return value;

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
    DEBUG_SERIAL.println("Toilet Paper Use Case");

    // Init Higrometer
    #if defined(ARDUINO_ARCH_SAMD)
        analogReadResolution(ANALOG_DEPTH);
    #endif
    pinMode(POT_PIN, INPUT);

    // Init radio
    wizeSetup();

}

void loop() {

    int16_t angle = getAngle();

    char payload[16];
    snprintf(payload, sizeof(payload), "%d,%d", angle, angle > ANGLE_THRESHOLD ? 1 : 0);
    wizeSend(payload);

    sleep(SLEEP_TIME);

}
