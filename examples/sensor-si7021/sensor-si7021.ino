/*

Allwize - SI7021/HTU21C Slave Example

Shows how to use an SI7021 or HTU21C sensor (temperature & humidity)
to send environmental data.

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

#if defined(ARDUINO_AVR_UNO)
    #define RESET_PIN   7
    #define RX_PIN      8
    #define TX_PIN      9
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
    #define debug       Serial
#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)
    #define RESET_PIN   7
    #define module      Serial1
    #define debug       Serial
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define RESET_PIN   7
    #define module      Serial1
    #define debug       SerialUSB
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)
    #define RESET_PIN   14
    #define RX_PIN      12
    #define TX_PIN      13
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
    #define debug       Serial
#endif // ARDUINO_ARCH_ESP8266


// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL        CHANNEL_04
#define WIZE_POWER          POWER_20dBm
#define WIZE_DATARATE       DATARATE_2400bps
#define WIZE_NETWORK_ID     0x46
#define WIZE_NODE_ID        0x11

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
// Formatting
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

void wizeSetup() {

    allwize = new Allwize(module, RESET_PIN);
    allwize->reset();
    module.begin(19200);
    while (!allwize->ready());
    allwize->begin();

    allwize->slave();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);
    allwize->setControlField(WIZE_NETWORK_ID);
    allwize->setControlInformation(WIZE_NODE_ID);

}

void wizeSend(const char * payload) {

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
