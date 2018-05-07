/*

Allwize - Memory Dump Example

This example uses a wrapping class around Allwize class to
expose the getMemory method and dump the radio module memory
byte by byte.
This is possible since all methods in the AllWize class are either
public or protected.

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
// Allwize class wrapper
// -----------------------------------------------------------------------------

#include "Allwize.h"

class AllwizeWrap : public Allwize {
    public:
        AllwizeWrap(Stream& stream, uint8_t reset_gpio = 0xFF): Allwize(stream, reset_gpio) {};
        uint8_t getMemory(uint8_t address) { return _getMemory(address); }

};

AllwizeWrap * allwize;

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    Serial.begin(115200);
    delay(5000);

    Serial.println();
    Serial.println("Allwize - Module Memory Dump");
    Serial.println();

    module.begin(19200);
    module.setTimeout(2000);
    allwize = new AllwizeWrap(module);
    allwize->setTimeout(2000);

    char buffer[10];

    Serial.print("      ");
    for (uint16_t address = 0; address <= 0x0F; address++) {
        snprintf(buffer, sizeof(buffer), " %02X", address);
        Serial.print(buffer);
    }
    Serial.println();
    Serial.print("------------------------------------------------------");

    for (uint16_t address = 0; address <= 0xFF; address++) {
        if ((address % 16) == 0) {
            snprintf(buffer, sizeof(buffer), "\n0x%02X: ", address);
            Serial.print(buffer);
        }
        snprintf(buffer, sizeof(buffer), " %02X", allwize->getMemory(address));
        Serial.print(buffer);
    }

    Serial.println();
    Serial.println();
    Serial.println("Done");

}

void loop() {}
