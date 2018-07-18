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
// Configuration
// -----------------------------------------------------------------------------

// Define whether we should use 0/1 or 8/9 pins to communicate
// with the radio module if this option is available.
// Some boards will only allow to use 8/9 if we want to have
// USB serial with the computer
#ifndef USE_SOFTWARE_SERIAL
#define USE_SOFTWARE_SERIAL             0
#endif

// -----------------------------------------------------------------------------
// Board definitions
// -----------------------------------------------------------------------------

// USB serial will depend on the board platform
#if defined(ARDUINO_SAMD_ZERO)
    #define DEBUG_SERIAL                SerialUSB
#else
    #define DEBUG_SERIAL                Serial
#endif

// The Arduino UNO has just one hardware serial, so communication
// with the module will always be using SoftwareSerial library
#if defined(ARDUINO_AVR_UNO)
    #define RESET_PIN                   7
    #define RX_PIN                      8
    #define TX_PIN                      9
#endif // ARDUINO_AVR_UNO

// The Arduino Leonardo has a dedicated serial for USB (Serial)
// We can use the other hradware serial (Serial1) to communicate with
// the radio module or use SoftwareSerial library
#if defined(ARDUINO_AVR_LEONARDO)
    #define RESET_PIN                   7
    #if USE_SOFTWARE_SERIAL
        #define RX_PIN                  8
        #define TX_PIN                  9
    #else
        #define HARDWARE_SERIAL         Serial1
    #endif
#endif // ARDUINO_AVR_LEONARDO

// The SAMD has 3 configurable hardware UART, so we can use both options too
#if defined(ARDUINO_SAMD_ZERO)
    #define RESET_PIN                   8
    #if USE_SOFTWARE_SERIAL
        #define RX_PIN                  7
        #define TX_PIN                  6
        #include "wiring_private.h"
        Uart Serial3(&sercom3, RX_PIN, TX_PIN, SERCOM_RX_PAD_3, UART_TX_PAD_2);
        void SERCOM3_Handler() { Serial3.IrqHandler(); }
        #define HARDWARE_SERIAL         Serial3
    #else
        #define HARDWARE_SERIAL         Serial1
    #endif
#endif // ARDUINO_ARCH_SAMD

// The ESP8266 Wemos D1 has just one hardware serial, so communication
// with the module will always be using SoftwareSerial library
#if defined(ARDUINO_ARCH_ESP8266)
    #define RESET_PIN                   14
    #define RX_PIN                      12
    #define TX_PIN                      13
#endif // ARDUINO_ARCH_ESP8266

// The ESP32 Wemos maps the USB to the first hardware serial in 0/1,
// so we only have the option to use a secondary hardware serial and
// configure it to pins 8 and 9 (GPIO12 and 13)
#if defined(ARDUINO_ARCH_ESP32)
    #define RESET_PIN                   14
    #define RX_PIN                      12
    #define TX_PIN                      13
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
    assertTrue(allwize->factoryReset());

    // get channel once more (factory channel is 3)
    uint8_t channel4 = allwize->getChannel();
    assertEqual((uint8_t) 3, channel4);

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void info() {

    DEBUG_SERIAL.println();

    #ifdef ARDUINO_AVR_UNO
        DEBUG_SERIAL.println("Board : Arduino UNO R3");
    #endif
    #ifdef ARDUINO_AVR_LEONARDO
        DEBUG_SERIAL.println("Board : Arduino Leonardo");
    #endif
    #ifdef ARDUINO_SAMD_ZERO
        DEBUG_SERIAL.println("Board : Arduino Zero / M0 / M0 Pro");
    #endif
    #ifdef ARDUINO_ARCH_ESP8266
        DEBUG_SERIAL.println("Board : ESP8266");
    #endif
    #ifdef ARDUINO_ARCH_ESP32
        DEBUG_SERIAL.println("Board : ESP32");
    #endif

    #if USE_SOFTWARE_SERIAL
        DEBUG_SERIAL.println("Serial: Pins 8 & 9");
    #else
        DEBUG_SERIAL.println("Serial: Pins 0 & 1");
    #endif

    DEBUG_SERIAL.println();

}

void setup() {

    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL);

    info();

    // Create and init AllWize object
    #if defined(HARDWARE_SERIAL)
        allwize = new Allwize(&HARDWARE_SERIAL, RESET_PIN);
    #else
        allwize = new Allwize(RX_PIN, TX_PIN, RESET_PIN);
    #endif
    allwize->begin();
    if (!allwize->waitForReady()) {
        DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        while (true);
    }

    Printer::setPrinter(&DEBUG_SERIAL);
    TestRunner::setVerbosity(Verbosity::kAll);

}

void loop() {
    TestRunner::run();
}
