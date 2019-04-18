/*

AllWize - Pollution Use Case
Using a MICS-4514 sensor

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

#define PRE_PIN             8
#define NOX_PIN             A0
#define RED_PIN             A1
#define CALIB_R0_NO2        2200      // R0 calibration value for the NO2 sensor
#define CALIB_R0_CO         750000    // R0 calibration value for the CO sensor

#define PRE_HEAT_TIME       10000   // in ms
#define SLEEP_TIME          5000    // in ms
#define SAMPLE_COUNT        10
#define ANALOG_MAX          (1 << ANALOG_DEPTH)

// -----------------------------------------------------------------------------
// State
// -----------------------------------------------------------------------------

uint16_t co;
uint16_t no2;

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

void read_MICS_4514() {

    uint16_t reading;
    float volt, res;

    // ------------
    // CO sensor
    // ------------

    reading = analogRead(RED_PIN);
    volt = (reading * 5.0) / ANALOG_MAX;
    res = (1 - volt / 5.0) * CALIB_R0_CO / 1000;

    DEBUG_SERIAL.print("[Sensor] R(CO): ");
    DEBUG_SERIAL.println(res);
    co = 10 * res;

    // Convert to ppm
    /*
    if (res > 0.7) res = 0.7;
    if (res > 0.6) {
        co = (0.711 - res) / 0.011;
    } else if (res > 0.3) {
        co = (0.7 - res) / 0.01;
    } else {
        co = (0.3233 - res) / 0.00058;
    }
    */

    // ------------
    // NO2 sensor
    // ------------

    reading = analogRead(NOX_PIN);
    volt = (reading * 5.0) / ANALOG_MAX;
    res = (1 - volt / 5.0) * CALIB_R0_NO2 / 1000;

    DEBUG_SERIAL.print("[Sensor] R(NO2): ");
    DEBUG_SERIAL.println(res);
    no2 = 10 * res;

    // Convert to ppm
    /*
    if (res < 3.0) res = 3.0;
    if (res >= 3.0 && res < 8.0) {
        no2 = (res - 0.5) / 0.25;
    } else {
        no2 = (res + 129.655) / 4.589;
    }
    */

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
    DEBUG_SERIAL.println("Pollution Use Case");

    // Init Sensor
    #if defined(ARDUINO_ARCH_SAMD)
        analogReadResolution(ANALOG_DEPTH);
    #endif
    pinMode(RED_PIN, INPUT);
    pinMode(NOX_PIN, INPUT);
    pinMode(PRE_PIN, OUTPUT);
    DEBUG_SERIAL.print("Preheating sensor... ");
    digitalWrite(PRE_PIN, 1);
    delay(PRE_HEAT_TIME);
    digitalWrite(PRE_PIN, 0);
    DEBUG_SERIAL.println("Done");

    // Init radio
    wizeSetup();

}

void loop() {

    read_MICS_4514();

    char payload[16];
    snprintf(payload, sizeof(payload), "%u,%u", co, no2);
    wizeSend(payload);

    sleep(SLEEP_TIME);

}
