/*

AllWize - SI7021/HTU21C Slave Example

Shows how to use an SI7021 or HTU21C sensor (temperature & humidity)
to send environmental data.

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
#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>

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
    #define MODULE_SERIAL        Serial1
    #define DEBUG_SERIAL        Serial
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define RESET_PIN           7
    #define MODULE_SERIAL       Serial1
    #define DEBUG_SERIAL        SerialUSB
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)
    #define RESET_PIN           14
    #define RX_PIN              5
    #define TX_PIN              4
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
#define WIZE_UID                0x20212223

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#if defined(MODULE_SERIAL)
    AllWize allwize(&MODULE_SERIAL, RESET_PIN);
#else
    AllWize allwize(RX_PIN, TX_PIN, RESET_PIN);
#endif

Weather sensor;

// -----------------------------------------------------------------------------
// I2C utils
// -----------------------------------------------------------------------------

bool i2cCheck(uint8_t address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission();
}

void i2cScan() {

    Wire.begin();

    char buffer[64];
    uint8_t count = 0;

    for (uint8_t address = 1; address < 127; address++) {
        uint8_t error = i2cCheck(address);
        if (error == 0) {
            snprintf(
                buffer, sizeof(buffer),
                "[I2C] Device found at address 0x%02X",
                address
            );
            DEBUG_SERIAL.println(buffer);
            count++;
        }
    }

    snprintf(buffer, sizeof(buffer), "[I2C] %u device found", count);
    DEBUG_SERIAL.println(buffer);

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
// AllWize
// -----------------------------------------------------------------------------

void wizeSetup() {

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

}

void wizeSend(const char * payload) {

    DEBUG_SERIAL.print("[AllWize] Payload: ");
    DEBUG_SERIAL.println(payload);

    if (!allwize.send(payload)) {
        DEBUG_SERIAL.println("[AllWize] Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Init DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[AllWize] SI7021 sensor example");

    delay(100);
    i2cScan();

    // Init SI7021 sensor
    sensor.begin();

    // Init radio
    wizeSetup();

}

void loop() {

    double t_n = sensor.getTemp();
    uint8_t h_n = sensor.getRH();

    char t_s[7];
    snfloat(t_s, sizeof(t_s), 1, t_n);

    char payload[20];
    snprintf(payload, sizeof(payload), "%s,%u", t_s, h_n);

    wizeSend(payload);

    delay(5000);

}
