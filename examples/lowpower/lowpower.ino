/*

Allwize - Low Power Example

Pretty-prints out the configuration settings stored in the module non-volatile memory.

Copyright (C) 2018-2019 by AllWize <github@allwize.io>

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
#include "wiring_private.h"
#if defined(ARDUINO_ARCH_SAMD)
    #include "ArduinoLowPower.h"
#else
    #include "LowPower.h"
#endif

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_14dBm
#define WIZE_DATARATE           DATARATE_2400bps
#define WIZE_UID                0x20212223

#define RADIO_SLEEP             1
#define MCU_SLEEP               0

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
    #define DEBUG_SERIAL        SerialUSB

    #if defined(ARDUINO_ALLWIZE_K2)
        
        #define RESET_PIN       PIN_WIZE_RESET
        #define MODULE_SERIAL   SerialWize

    #else
        
        // Arduino M0 / M0Pro
        #define RESET_PIN       7
        #define MODULE_SERIAL   Serial1

        // Custom configuration combinations:
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
        /*
        #define RESET_PIN           (30ul)
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
        */

    #endif

#endif // ARDUINO_ARCH_SAMD

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#if defined(MODULE_SERIAL)
    AllWize allwize(&MODULE_SERIAL, RESET_PIN);
#else
    AllWize allwize(RX_PIN, TX_PIN, RESET_PIN);
#endif

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

void wizeSetup() {

    #if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.println("Initializing radio module");
    #endif

    #if defined(ARDUINO_ARCH_SAMD) && defined(RX_PIN) && defined(TX_PIN)
        pinPeripheral(RX_PIN, SERCOM_MODE);
        pinPeripheral(TX_PIN, SERCOM_MODE);
    #endif

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        #if defined(DEBUG_SERIAL)
            DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        #endif
        while (true);
    }

    allwize.slave();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);
    allwize.setUID(WIZE_UID);

    #if defined(DEBUG_SERIAL)
        allwize.dump(DEBUG_SERIAL);
        DEBUG_SERIAL.println("[WIZE] Ready...");
    #endif

}

void wizeSend(uint8_t * payload, size_t len) {

    char buffer[64];
    DEBUG_SERIAL.print("[WIZE] Sending: ");
    for (uint8_t i = 0; i<len; i++) {
        snprintf(buffer, sizeof(buffer), "%02X", payload[i]);
    DEBUG_SERIAL.print(buffer);
    }
    DEBUG_SERIAL.print("\n");

    if (!allwize.send(payload, len)) {
        DEBUG_SERIAL.println("[WIZE] Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Sleep
// -----------------------------------------------------------------------------

void sleep() {

    // Sleep the radio
    #if RADIO_SLEEP
        allwize.sleep();
        delay(10);
    #endif

    // Sleep the MCU
    #if MCU_SLEEP
        #if defined(ARDUINO_ARCH_SAMD)
            LowPower.sleep(8000);
        #else
            LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
        #endif
    #else
        delay(8000);
    #endif

}

void wakeup() {

    // Wake up the radio
    #if RADIO_SLEEP
        allwize.wakeup();
    #endif

    #if MCU_SLEEP

        // Wait for debug port to be enabled
        #if defined(DEBUG_SERIAL)
            //while(!DEBUG_SERIAL);
        #endif
    
    #endif
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Init DEBUG_SERIAL
    #if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.begin(115200);
        while (!DEBUG_SERIAL && millis() < 2000);
        DEBUG_SERIAL.println();
        DEBUG_SERIAL.println("Allwize - Low Power");
        DEBUG_SERIAL.println();
    #endif

    // -------------------------------------------------------------------------

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    wizeSetup();

    #if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.println("Ready");
    #endif

}

void loop() {

    // -------------------------------------------------------------------------

    digitalWrite(LED_BUILTIN, HIGH);
    uint8_t payload[1] = { 0x55 };
    wizeSend(payload, sizeof(payload));
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);

    // -------------------------------------------------------------------------

    #if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.println("Going into low-power mode for 8s");
    #endif
    delay(10);
    sleep();
    wakeup();

}
