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
#define USE_SOFTWARE_SERIAL        0
#endif

// -----------------------------------------------------------------------------
// Board definitions
// -----------------------------------------------------------------------------

// USB serial will depend on the board platform
#if defined(ARDUINO_SAMD_ZERO)
    #define debug   SerialUSB
#else
    #define debug   Serial
#endif

// The Arduino UNO has just one hardware serial, so communication
// with the module will always be using SoftwareSerial library
#if defined(ARDUINO_AVR_UNO)
    #define RX_PIN              8
    #define TX_PIN              9
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
#endif // ARDUINO_AVR_UNO

// The Arduino Leonardo has a dedicated serial for USB (Serial)
// We can use the other hradware serial (Serial1) to communicate with
// the radio module or use SoftwareSerial library
#if defined(ARDUINO_AVR_LEONARDO) || defined(ARDUINO_AVR_YUN)
    #if USE_SOFTWARE_SERIAL
        #define RX_PIN          8
        #define TX_PIN          9
        #include <SoftwareSerial.h>
        SoftwareSerial module(RX_PIN, TX_PIN);
    #else
        #define module          Serial1
    #endif
#endif // ARDUINO_AVR_LEONARDO

// The SAMD has 3 configurable hardware UART, so we can use both options too
#if defined(ARDUINO_SAMD_ZERO)
    #if USE_SOFTWARE_SERIAL
        #define RX_PIN          8
        #define TX_PIN          9
        #include "wiring_private.h"
        Uart module(&sercom3, RX_PIN, TX_PIN, SERCOM_RX_PAD_3, UART_TX_PAD_2);
        void SERCOM3_Handler() { module.IrqHandler(); }
    #else
        #define module          Serial1
    #endif
#endif // ARDUINO_ARCH_SAMD

// The ESP8266 Wemos D1 has just one hardware serial, so communication
// with the module will always be using SoftwareSerial library
#if defined(ARDUINO_ARCH_ESP8266)
    #define RX_PIN              12
    #define TX_PIN              13
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
#endif // ARDUINO_ARCH_ESP8266

// The ESP32 Wemos maps the USB to the first hardware serial in 0/1,
// so we only have the option to use a secondary hardware serial and
// configure it to pins 8 and 9 (GPIO12 and 13)
#if defined(ARDUINO_ARCH_ESP32)
    #define RX_PIN              12
    #define TX_PIN              13
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
    assertTrue(allwize->factoryReset());

    // We must reset the serial connection after a reset or factoryReset
    module.end();
    #if defined(ARDUINO_ARCH_ESP32)
        module.begin(19200, SERIAL_8N1, RX_PIN, TX_PIN);
    #else
        module.begin(19200);
    #endif
    delay(200);

    // get channel once more (factory channel is 3)
    uint8_t channel4 = allwize->getChannel();
    assertEqual((uint8_t) 3, channel4);

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void info() {

    debug.println();

    #ifdef ARDUINO_AVR_UNO
        debug.println("Board : Arduino UNO R3");
    #endif
    #ifdef ARDUINO_AVR_LEONARDO
        debug.println("Board : Arduino Leonardo");
    #endif
    #ifdef ARDUINO_AVR_YUN
        debug.println("Board : Arduino Yun");
    #endif
    #ifdef ARDUINO_SAMD_ZERO
        debug.println("Board : Arduino Zero / M0 / M0 Pro");
    #endif
    #ifdef ARDUINO_ARCH_ESP8266
        debug.println("Board : ESP8266");
    #endif
    #ifdef ARDUINO_ARCH_ESP32
        debug.println("Board : ESP32");
    #endif

    #if USE_SOFTWARE_SERIAL
        debug.println("Serial: Pins 8 & 9");
    #else
        debug.println("Serial: Pins 0 & 1");
    #endif

    debug.println();

}

void setup() {

    debug.begin(115200);
    while (!debug);

    info();

    #if defined(ARDUINO_ARCH_ESP32)
        pinMode (RX_PIN, FUNCTION_4);
        pinMode (TX_PIN, FUNCTION_4);
        module.begin(19200, SERIAL_8N1, RX_PIN, TX_PIN);
    #elif defined(ARDUINO_ARCH_SAMD) && USE_SOFTWARE_SERIAL
        pinPeripheral(RX_PIN, PIO_SERCOM);
        pinPeripheral(TX_PIN, PIO_SERCOM);
        module.begin(19200);
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
