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

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_14dBm
#define WIZE_DATARATE           DATARATE_2400bps

#define RADIO_SLEEP             1
#define MCU_SLEEP               1

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
        #define SERCOM_PORT         sercom2
        #define SERCOM_HANDLER      SERCOM2_Handler
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

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

#include "AllWize.h"
AllWize * allwize;

void wizeSetup() {

    #if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.println("Initializing radio module");
    #endif

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
        #if defined(DEBUG_SERIAL)
            DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        #endif
        while (true);
    }

    allwize->slave();
    allwize->setMode(MBUS_MODE_OSP);
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);

    #if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.println("Radio module OK");
    #endif

}

void wizeSend(const char * payload) {

    #if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.print("[AllWize] Payload: ");
        DEBUG_SERIAL.println(payload);
    #endif

    if (!allwize->send(payload)) {
        #if defined(DEBUG_SERIAL)
            DEBUG_SERIAL.println("[AllWize] Error sending message");
        #endif
    }

}

// -----------------------------------------------------------------------------
// Sleep
// -----------------------------------------------------------------------------

#if defined(ARDUINO_ARCH_SAMD)
    #include "ArduinoLowPower.h"
#else
    #include "LowPower.h"
#endif

void sleep() {

    // Sleep the radio
    #if RADIO_SLEEP
        allwize->sleep();
        delay(10);
    #endif

    // Sleep the MCU
    #if MCU_SLEEP
        #if defined(ARDUINO_ARCH_SAMD)
            LowPower.sleep(20000);
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
        allwize->wakeup();
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
    wizeSend("U");
    #if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.println("Idle for 8s");
    #endif
    delay(8000);
    digitalWrite(LED_BUILTIN, LOW);

    // -------------------------------------------------------------------------

    #if defined(DEBUG_SERIAL)
        DEBUG_SERIAL.println("Going into low-power mode for 8s");
    #endif
    delay(10);
    sleep();
    wakeup();

}
