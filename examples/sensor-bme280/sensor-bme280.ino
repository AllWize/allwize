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

#if defined(ARDUINO_ARCH_SAMD)
    #define debug   SerialUSB
#else
    #define debug   Serial
#endif

#if defined(ARDUINO_AVR_LEONARDO)
    #define module      Serial1
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define module      Serial1
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_SAMD)

#include <stdio.h>

char *dtostrf (double val, signed char width, unsigned char prec, char *sout) {
  asm(".global _printf_float");

  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}

#endif

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define CHANNEL                 0x04
#define NETWORK_ID              0x46
#define NODE_ID                 0x09

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
Adafruit_BME280 sensor;

// -----------------------------------------------------------------------------
// I2C utils
// -----------------------------------------------------------------------------

bool i2cCheck(unsigned char address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission();
}

void i2cScan() {
    unsigned char nDevices = 0;
    char buffer[64];
    for (unsigned char address = 1; address < 127; address++) {
        unsigned char error = i2cCheck(address);
        if (error == 0) {
            snprintf(
                buffer, sizeof(buffer),
                "[I2C] Device found at address 0x%02X",
                address
            );
            debug.println(buffer);
            nDevices++;
        }
    }
    if (nDevices == 0) debug.println("[I2C] No devices found\n");
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
    debug.println("[Allwize] BME280 sensor example");

    // Init BME280 sensor
    if (!sensor.begin(0x40)) {
        debug.println("[Allwize] Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    debug.println("[Allwize] Sensor ready!");

    // Init radio
    radioSetup();

}

void loop() {

    double t_n = sensor.readTemperature();
    uint8_t h_n = sensor.readHumidity();
    double p_n = sensor.readPressure();

    char t_s[7]; dtostrf(t_n, sizeof(t_s), 2, t_s);
    char p_s[8]; dtostrf(p_n, sizeof(p_s), 2, p_s);

    char payload[20];
    snprintf(payload, sizeof(payload), "%s,%u,%s", t_s, h_n, p_s);

    radioSend(payload);

    delay(5000);

}
