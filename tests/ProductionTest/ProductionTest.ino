/*

AllWize - Production Test suite

This test suite tests the hardware by running a series of commands over a RC1701XX module.
This test suite uses Aunit unit testing framework (https://github.com/bxparks/AUnit)


Copyright (C) 2018-2020 by AllWize <github@allwize.io>

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
        #define MODULE_SERIAL         Serial1
    #endif
#endif // ARDUINO_AVR_LEONARDO

// The SAMD has 3 configurable hardware UART, so we can use both options too
#if defined(ARDUINO_ARCH_SAMD)

    // Common:
    #define DEBUG_SERIAL        SerialUSB

    // Configuring additional hardware serials:
    // Possible combinations:
    //
    // SERCOM1:
    //    RX on 10,11,12,13
    //    TX on 10,11
    //    Mode PIO_SERCOM
    //
    // SERCOM3:
    //    RX on 6,7,10,11,12,13
    //    TX on 6,10,11
    //    Mode PIO_SERCOM_ALT
    //    6-10 and 7-12 are not compatible
    //
    // Pads:
    //    6   pad 2
    //    7   pad 3 (only RX)
    //    10  pad 2
    //    11  pad 0
    //    12  pad 3 (only RX)
    //    13  pad 1 (only RX)

    #if defined(ALLWIZE_K2)

        #define RX_PIN              (29ul)
        #define TX_PIN              (26ul)
        #define SERCOM_PORT         sercom4
        #define SERCOM_HANDLER      SERCOM4_Handler
        #define SERCOM_MODE         PIO_SERCOM_ALT
        #define SERCOM_RX_PAD       SERCOM_RX_PAD_3
        #define SERCOM_TX_PAD       UART_TX_PAD_0
        #include "wiring_private.h" // pinPeripheral() function
        Uart SerialWize(&SERCOM_PORT, RX_PIN, TX_PIN, SERCOM_RX_PAD, SERCOM_TX_PAD);
        void SERCOM_HANDLER() { SerialWize.IrqHandler(); }
        #define MODULE_SERIAL       SerialWize
        #define RESET_PIN           (30u)

    #else

        // Using exposed hardware serials:
        #define RESET_PIN           7
        #define MODULE_SERIAL       Serial1

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

#include "AllWize.h"
AllWize * allwize;

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

test(factoryreset) {

    // change channel
    allwize->setChannel(CHANNEL_04, true);

    // factory reset
    assertTrue(allwize->factoryReset());

    // get channel once more (factory channel is 3)
    uint8_t value = allwize->getChannel();
    assertEqual((uint8_t) CHANNEL_03, value);

}

test(setchannel) {

    // get original channel
    uint8_t channel1 = allwize->getChannel();

    // set new channel
    uint8_t channel2 = channel1 + 1;
    allwize->setChannel(channel2, true);

    // get channel again
    uint8_t channel3 = allwize->getChannel();
    assertEqual(channel2, channel3);

}

test(setbaudrate) {

    // get current channel
    uint8_t channel1 = allwize->getChannel();

    // change baudrate and reset
    allwize->setBaudRate(BAUDRATE_115200);
    assertEqual(BAUDRATE_115200, allwize->getBaudRate());
    allwize->reset();

    // query baudrate
    assertEqual(BAUDRATE_115200, allwize->getBaudRate());

    // query channel again
    uint8_t channel2 = allwize->getChannel();
    assertEqual(channel1, channel2);

    // change baudrate again
    allwize->setBaudRate(MODEM_DEFAULT_BAUDRATE);
    allwize->reset();

    // query baudrate
    assertEqual(MODEM_DEFAULT_BAUDRATE, allwize->getBaudRate());

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

    #if defined(RX_PIN) && defined(TX_PIN)
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Serial: Pins %d & %d", (int) RX_PIN, (int) TX_PIN);
        DEBUG_SERIAL.println(buffer);
    #else
        DEBUG_SERIAL.println("Serial: Pins 0 & 1");
    #endif

    DEBUG_SERIAL.println();

}

void setup() {

    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL);

    info();

    #if defined(ARDUINO_ARCH_SAMD) && defined(RX_PIN) && defined(TX_PIN)
        pinPeripheral(RX_PIN, SERCOM_MODE);
        pinPeripheral(TX_PIN, SERCOM_MODE);
    #endif

    // Create and init AllWize object
    #if defined(MODULE_SERIAL)
        allwize = new AllWize(&MODULE_SERIAL, RESET_PIN);
    #else
        allwize = new AllWize(RX_PIN, TX_PIN, RESET_PIN);
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
