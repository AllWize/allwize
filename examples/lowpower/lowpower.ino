/*

Allwize - Low Power Example

Pretty-prints out the configuration settings stored in the module non-volatile memory.

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
// Config & globals
// -----------------------------------------------------------------------------

#include "Allwize.h"
Allwize * allwize;

// -----------------------------------------------------------------------------
// Board definitions
// -----------------------------------------------------------------------------

#if defined(ARDUINO_AVR_UNO)

    // Common
    #define RESET_PIN           7
    #define DEBUG_SERIAL        Serial

    // Using software serial:
    #define RX_PIN              8
    #define TX_PIN              9

#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)

    // Common:
    #define RESET_PIN           7
    #define DEBUG_SERIAL        Serial

    // Using hardware serial:
    #define MODULE_SERIAL       Serial1


#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)

    // Common:
    #define RESET_PIN           7
    #define DEBUG_SERIAL        SerialUSB

    // Using exposed hardware serials:
    #define MODULE_SERIAL       Serial1


#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)

    // Common:
    #define RESET_PIN           14
    #define DEBUG_SERIAL        Serial

    // Using Software serial:
    #define RX_PIN              12
    #define TX_PIN              13

#endif // ARDUINO_ARCH_ESP8266

#if defined(ARDUINO_ARCH_ESP32)

    // Common:
    #define RESET_PIN           14
    #define DEBUG_SERIAL        Serial

    // Using Hardware serial on random GPIOs:
    #define RX_PIN              12
    #define TX_PIN              13

#endif // ARDUINO_ARCH_ESP32

// -----------------------------------------------------------------------------
// Radio
// -----------------------------------------------------------------------------

void wizeSetup() {

    DEBUG_SERIAL.println("Checking radio module");

    #if defined(ARDUINO_ARCH_SAMD) && defined(RX_PIN) && defined(TX_PIN)
        pinPeripheral(RX_PIN, SERCOM_MODE);
        pinPeripheral(TX_PIN, SERCOM_MODE);
    #endif

    // Create and init AllWize object
    #if defined(MODULE_SERIAL)
        allwize = new Allwize(&MODULE_SERIAL, RESET_PIN);
    #else
        allwize = new Allwize(RX_PIN, TX_PIN, RESET_PIN);
    #endif

    allwize->begin();

    if (!allwize->waitForReady()) {
        DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        while (true);
    }

    DEBUG_SERIAL.println("Radio module OK");

}

// -----------------------------------------------------------------------------
// Sleep
// -----------------------------------------------------------------------------

#include "LowPower.h"

#if defined(ARDUINO_ARCH_SAMD)

#include <RTCZero.h>
RTCZero rtc;
unsigned long datetime = 1451606400;

void isr() {}

void rtcSetup() {
    rtc.begin();
    rtc.setEpoch(datetime);
    rtc.attachInterrupt(isr);
}

#endif

void sleep() {

    allwize->sleep();

    #if defined(ARDUINO_ARCH_SAMD)
        rtc.setEpoch(datetime);
        rtc.setAlarmEpoch(datetime + 2);
        rtc.enableAlarm(rtc.MATCH_HHMMSS);
        //USBDevice.detach();
        LowPower.standby();
    #else
        LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
    #endif

}

void wakeup() {

    allwize->wakeup();

    #if defined(ARDUINO_ARCH_SAMD)
        //USBDevice.attach();
    #endif

    while(!SerialUSB);

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("Allwize - Low Power");
    DEBUG_SERIAL.println();

    // -------------------------------------------------------------------------

    wizeSetup();

    #if defined(ARDUINO_ARCH_SAMD)
    rtcSetup();
    #endif

}

void loop() {

    // -------------------------------------------------------------------------

    DEBUG_SERIAL.println("Idle for 100s");
    delay(100);

    // -------------------------------------------------------------------------

    DEBUG_SERIAL.println("Going into low-power mode for 2s");
    sleep();
    wakeup();

}
