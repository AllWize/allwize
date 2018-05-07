/*

Allwize - Module Info Example

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

#if defined(ARDUINO_AVR_UNO)
    #define RX_PIN      8
    #define TX_PIN      9
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)
    #define module      Serial1
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define module      Serial1
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)
    #define RX_PIN      12
    #define TX_PIN      13
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
#endif // ARDUINO_ARCH_ESP8266

// -----------------------------------------------------------------------------
// Config & globals
// -----------------------------------------------------------------------------

#define COLUMN_PAD  20

#include "Allwize.h"
Allwize * allwize;

// -----------------------------------------------------------------------------
// Utils
// -----------------------------------------------------------------------------

void format(const char * name, const char * value) {
    Serial.print(name);
    for (uint8_t i=0; i<COLUMN_PAD-strlen(name); i++) Serial.print(" ");
    Serial.println(value);
}

void format(const char * name, String value) {
    Serial.print(name);
    for (uint8_t i=0; i<COLUMN_PAD-strlen(name); i++) Serial.print(" ");
    Serial.println(value);
}

void format(const char * name, int value) {
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", value);
    format(name, buffer);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    Serial.begin(115200);
    delay(5000);

    module.begin(19200);
    allwize = new Allwize(module);

    Serial.println();
    Serial.println("Allwize - Module Info");
    Serial.println();

    format("Property", "Value");
    Serial.println("------------------------------");

    format("Channel", allwize->getChannel());
    format("Power", allwize->getPower());
    format("MBUS Mode", allwize->getMBusMode());
    format("Sleep Mode", allwize->getSleepMode());
    format("Control Field", allwize->getControlField());

    format("Manufacturer ID", allwize->getMID());
    format("Unique ID", allwize->getUID());
    format("Version", allwize->getVersion());
    format("Device", allwize->getDevice());
    format("Part Number", allwize->getPartNumber());
    format("Hardware Version", allwize->getHardwareVersion());
    format("Firmware Version", allwize->getFirmwareVersion());
    format("Serial Number", allwize->getSerialNumber());

    format("Temperature (C)", allwize->getTemperature());
    format("Voltage (mV)", allwize->getVoltage());

}

void loop() {}
