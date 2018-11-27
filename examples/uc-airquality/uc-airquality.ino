/*

AllWize - Air Quality Use Case

MH-Z16 CO2 Sensor
BME-280 Temperature, Humidity & Pressure Sensor

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

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL        CHANNEL_11
#define WIZE_POWER          POWER_20dBm
#define WIZE_DATARATE       DATARATE_2400bps
#define WIZE_NODE_ID        0x0B

#define MHZ16_PWM           5

#define SEND_INTERVAL       5000

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
        DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize->slave();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);
    allwize->setControlInformation(WIZE_NODE_ID);

}

void wizeSend(const char * payload) {

    DEBUG_SERIAL.print("[AllWize] Payload: ");
    DEBUG_SERIAL.println(payload);

    if (!allwize->send(payload)) {
        DEBUG_SERIAL.println("[AllWize] Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Sensor setup
// -----------------------------------------------------------------------------

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme280;

void sensorSetup() {

    // MH-Z16 sensor
    pinMode(MHZ16_PWM, INPUT);

    // BME-280 sensor
    if (!bme280.begin(0x76)) {
        DEBUG_SERIAL.println("[AllWize] Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    DEBUG_SERIAL.println("[AllWize] Sensors ready!");

}

unsigned long getPPM() {
    unsigned long ms = pulseIn(MHZ16_PWM, HIGH, 2000000) / 1000;
    return (ms > 2) ? 2 * (ms - 2) : 0;
}

// -----------------------------------------------------------------------------
// Formatting
// -----------------------------------------------------------------------------

char * snfloat(char * buffer, uint8_t len, uint8_t decimals, float value) {

    bool negative = value < 0;
    if (negative) value = -value;

    uint32_t mul = 1;
    for (uint8_t i=0; i<decimals; i++) mul *= 10;

    uint32_t value_int = int(value);
    uint32_t value_dec = int(mul * (value - value_int));

    char format[20];
    snprintf(format, sizeof(format), "%s%%lu.%%0%ulu", negative ? "-" : "", decimals);
    snprintf(buffer, len, format, value_int, value_dec);

    return buffer;

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Init serial DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[AllWize] Air quality use case");
    DEBUG_SERIAL.println();

    // Init sensor
    sensorSetup();

    // Init radio
    wizeSetup();

}

void loop() {

    uint32_t c_n = getPPM();
    double t_n = (double) bme280.readTemperature();
    uint8_t h_n = bme280.readHumidity();
    double p_n = (double) bme280.readPressure() / 100.0;

    char t_s[7];
    snfloat(t_s, sizeof(t_s), 1, t_n);

    char p_s[8];
    snfloat(p_s, sizeof(p_s), 2, p_n);

    char payload[32];
    snprintf(payload, sizeof(payload), "%s,%u,%s,%lu", t_s, h_n, p_s, c_n);
    wizeSend(payload);

    delay(SEND_INTERVAL);

}
