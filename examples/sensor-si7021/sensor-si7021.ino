/*

Allwize - Sensor example

This example prints out the configuration settings stored
in the module non-volatile memory.

Copyright (C) 2018 by Allwize <github@allwize.io>

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

// These definitions are valid for SAMD and Leonardo boards
#define debug       SerialUSB
#define module      Serial1

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define CHANNEL                 0x04
#define NETWORK_ID              0x46
#define NODE_ID                 0x11

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>
Weather sensor;

// -----------------------------------------------------------------------------
// I2C utils
// -----------------------------------------------------------------------------

#include <Wire.h>

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
            debug.println(buffer);
            count++;
        }
    }

    snprintf(buffer, sizeof(buffer), "[I2C] %u device found", count);
    debug.println(buffer);

}

// -----------------------------------------------------------------------------
// Utils format
// -----------------------------------------------------------------------------

char * snfloat(char * buffer, size_t len, size_t decimals, float value) {

    bool negative = value < 0;

    uint32_t mul = 1;
    for (uint8_t i=0; i<decimals; i++) mul *= 10;

    value = abs(value);
    uint32_t value_int = int(value);
    uint32_t value_dec = int((value - value_int) * mul);

    char format[20];
    snprintf(format, sizeof(format), "%s%%lu.%%0%ulu", negative ? "-" : "", decimals);
    snprintf(buffer, len, format, value_int, value_dec);

    return buffer;

}

// -----------------------------------------------------------------------------
// Allwize
// -----------------------------------------------------------------------------

#include "Allwize.h"
Allwize * allwize;

void radioSetup() {

    module.begin(19200);
    allwize = new Allwize(module);
    allwize->begin();
    while (!allwize->ready());

    allwize->slave();
    allwize->setChannel(CHANNEL, true);
    allwize->setPower(5);
    allwize->setDataRate(1);
    allwize->setControlField(NETWORK_ID);
    allwize->setControlInformation(NODE_ID);

}

void radioSend(const char * payload) {

    debug.print("[Allwize] Payload: ");
    debug.println(payload);

    if (!allwize->send(payload)) {
        debug.println("[Allwize] Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Init debug
    debug.begin(115200);
    while (!debug && millis() < 5000);
    debug.println();
    debug.println("[Allwize] SI7021 sensor example");

    delay(100);
    i2cScan();

    // Init SI7021 sensor
    sensor.begin();

    // Init radio
    radioSetup();

}

void loop() {

    double t_n = sensor.getTemp();
    uint8_t h_n = sensor.getRH();

    char t_s[7];
    snfloat(t_s, sizeof(t_s), 1, t_n);

    char payload[20];
    snprintf(payload, sizeof(payload), "%s,%u", t_s, h_n);

    radioSend(payload);

    delay(5000);

}
