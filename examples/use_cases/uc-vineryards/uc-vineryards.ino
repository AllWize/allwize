/*

AllWize - Vineyard Case
Using an analog higrometer and a SI7021 temp & hum sensor

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

#include "AllWize.h"

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
    #define ANALOG_DEPTH        12
#endif // ARDUINO_ARCH_SAMD

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL        CHANNEL_04
#define WIZE_POWER          POWER_20dBm
#define WIZE_DATARATE       DATARATE_2400bps
#define WIZE_UID            0x20212223

#define HIGROMETER_PIN      A0
#define SLEEP_TIME          5000    // in ms
#define SAMPLE_COUNT        10
#define HIGROMETER_MIN      300     // Value when in water
#define ANALOG_MAX          ((1 << ANALOG_DEPTH) - 1)

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

AllWize allwize(&MODULE_SERIAL, RESET_PIN);

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

void wizeSetup() {

    DEBUG_SERIAL.println("Checking radio module");

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize.slave();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);
    allwize.setUID(WIZE_UID);

    DEBUG_SERIAL.println("Radio module OK");

}

void wizeSend(const char * payload) {

    DEBUG_SERIAL.print("Payload: ");
    DEBUG_SERIAL.println(payload);

    if (!allwize.send(payload)) {
        DEBUG_SERIAL.println("Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Sensors
// -----------------------------------------------------------------------------

#include <Wire.h>
#include <HTU21D.h>
HTU21D htu21d(HTU21D_RES_RH12_TEMP14);

uint16_t getHumidity() {

    double value = 0;
    for (unsigned char i=0; i<SAMPLE_COUNT; i++) {
        value += analogRead(HIGROMETER_PIN);
    }
    value = value / SAMPLE_COUNT;

    // Uncomment this line to get a reading of the sensor immersed in water,
    // set HIGROMETER_MIN to this value
    //Serial.print("Sensor reading: "); Serial.println(value);

    value = map(value, HIGROMETER_MIN, ANALOG_MAX, 100, 0);
    return constrain(value, 0, 100);

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
    DEBUG_SERIAL.println("Vineyard Use Case");

    // Init Higrometer
    #if defined(ARDUINO_ARCH_SAMD)
        analogReadResolution(ANALOG_DEPTH);
    #endif
    pinMode(HIGROMETER_PIN, INPUT);

    // Init SI7021
    if (!htu21d.begin()) {
        DEBUG_SERIAL.println("HTU21D sensor not found!");
        while (true);
    }

    // Init radio
    wizeSetup();

}

void loop() {

    char payload[16];
    snprintf(payload, sizeof(payload), "%d,%d,%d", (int) htu21d.readHumidity(), (int) htu21d.readTemperature(), getHumidity());
    wizeSend(payload);

    sleep(SLEEP_TIME);

}
