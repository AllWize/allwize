/*

Allwize - Memory Dump Example

This example dumps the radio module memory byte by byte.

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
    #define debug       Serial
#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)
    #define module      Serial1
    #define debug       Serial
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define module      Serial1
    #define debug       SerialUSB
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)
    #define RX_PIN      12
    #define TX_PIN      13
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
    #define debug       Serial
#endif // ARDUINO_ARCH_ESP8266

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#include "Allwize.h"
Allwize * allwize;

// -----------------------------------------------------------------------------
// Code
// -----------------------------------------------------------------------------

void setup() {

    debug.begin(115200);
    while (!debug && millis() < 5000);

    module.begin(19200);
    allwize = new Allwize(module);
    allwize->begin();
    while (!allwize->ready());

    debug.println();
    debug.println("Allwize - Module Memory Dump");
    debug.println();

    allwize->dump(debug);

    debug.println("Done");

}

void loop() {}
