/*

Allwize - Low Power Example

Pretty-prints out the configuration settings stored in the module non-volatile memory.

Copyright (C) 2018 by AllWize <github@allwize.io>

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
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL        CHANNEL_11
#define WIZE_POWER          POWER_14dBm
#define WIZE_DATARATE       DATARATE_2400bps
#define WIZE_NODE_ID        0x09

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
    #define HARDWARE_SERIAL     Serial1


#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)

    // Common:
    #define RESET_PIN           7
    #define DEBUG_SERIAL        SerialUSB

    // Using exposed hardware serials:
    #define HARDWARE_SERIAL     Serial1


#endif // ARDUINO_ARCH_SAMD

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

#include "AllWize.h"
AllWize * allwize;

void wizeSetup() {

    // Create and init AllWize object
    #if defined(HARDWARE_SERIAL)
        allwize = new AllWize(&HARDWARE_SERIAL, RESET_PIN);
    #else
        allwize = new AllWize(RX_PIN, TX_PIN, RESET_PIN);
    #endif
    allwize->begin();
    if (!allwize->waitForReady()) {
        DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize->slave();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);
    allwize->setControlInformation(WIZE_NODE_ID);

}

void wizeSend(const char * payload) {

    DEBUG_SERIAL.print("[AllWize] Payload: ");
    DEBUG_SERIAL.println(payload);

    if (!allwize->send(payload)) {
        DEBUG_SERIAL.println("[AllWize] Error sending message");
    }

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

    // Sleep the radio
    allwize->sleep();
    delay(10);

    // Sleep the MCU
    #if defined(ARDUINO_ARCH_SAMD)
        rtc.setEpoch(datetime);
        rtc.setAlarmEpoch(datetime + 8);
        rtc.enableAlarm(rtc.MATCH_HHMMSS);
        USBDevice.detach();
        LowPower.standby();
    #else
    	LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    #endif

}

void wakeup() {

    // Wake up the radio
    allwize->wakeup();

    // Wake up the MCU peripherials
    #if defined(ARDUINO_ARCH_SAMD)
        USBDevice.attach();
    #endif
    
    // Wait for debug port to be enabled
    while(!DEBUG_SERIAL);

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Init DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 2000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("Allwize - Low Power");
    DEBUG_SERIAL.println();

    // -------------------------------------------------------------------------

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    wizeSetup();

    #if defined(ARDUINO_ARCH_SAMD)
        rtcSetup();
    #endif

    DEBUG_SERIAL.println("Ready");

}

void loop() {

    // -------------------------------------------------------------------------

    digitalWrite(LED_BUILTIN, HIGH);
    wizeSend("U");
    DEBUG_SERIAL.println("Idle for 8s");
    delay(8000);
    digitalWrite(LED_BUILTIN, LOW);

    // -------------------------------------------------------------------------

    DEBUG_SERIAL.println("Going into low-power mode for 8s");
    delay(10);
    sleep();
    wakeup();

}
