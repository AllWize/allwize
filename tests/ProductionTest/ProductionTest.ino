/*

Allwize - Production Test suite

This test suite tests the hardware by running a series of commands over a RC1701XX module.
This test suite uses Aunit unit testing framework (https://github.com/bxparks/AUnit)


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

#include "Allwize.h"

#include "AUnit.h"
using namespace aunit;

// -----------------------------------------------------------------------------
// Board definitions
// -----------------------------------------------------------------------------

#if defined(ARDUINO_ARCH_SAMD)
    #define debug   SerialUSB
#else
    #define debug   Serial
#endif

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

#if defined(ARDUINO_ARCH_ESP32)
    #define RX_PIN      12
    #define TX_PIN      13
    HardwareSerial module(1);
#endif // ARDUINO_ARCH_ESP8266

// -----------------------------------------------------------------------------
// Config & globals
// -----------------------------------------------------------------------------

#define COLUMN_PAD  20

#include "Allwize.h"
Allwize * allwize;

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test(full) {

    // get original channel
    uint8_t channel1 = allwize->getChannel();
    //assertNotEqual((uint8_t) 0, channel1);

    // set new channel
    uint8_t channel2 = channel1 + 1;
    allwize->setChannel(channel2, true);

    // get channel again
    uint8_t channel3 = allwize->getChannel();
    assertEqual(channel2, channel3);

    // factory reset
    allwize->factoryReset();

    // We must reset the serial connection after a reset or factoryReset
    module.end();
    #if defined(ARDUINO_ARCH_ESP32)
        module.begin(19200, SERIAL_8N1, RX_PIN, TX_PIN);
    #else
        module.begin(19200);
    #endif
    module.flush();
    delay(200);

    // get channel once more (factory channel is 3)
    uint8_t channel4 = allwize->getChannel();
    assertEqual((uint8_t) 3, channel4);

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    debug.begin(115200);
    while (!debug);

    #if defined(ARDUINO_ARCH_ESP32)
        pinMode (RX_PIN, FUNCTION_4);
        pinMode (TX_PIN, FUNCTION_4);
        module.begin(19200, SERIAL_8N1, RX_PIN, TX_PIN);
    #else
        module.begin(19200);
    #endif
    allwize = new Allwize(module);

    Printer::setPrinter(&debug);
    TestRunner::setVerbosity(Verbosity::kAll);

}

void loop() {
    TestRunner::run();
}
