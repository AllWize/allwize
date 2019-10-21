/*

AllWize - WIZE 2 Serial Bridge

Listens to messages on the same channel, data rate and CF and
forwards them via serial to a listening service on a connected computer.

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

// -----------------------------------------------------------------------------
// Board definitions
// -----------------------------------------------------------------------------

#if defined(ARDUINO_AVR_UNO)
    #define RESET_PIN           7
    #define RX_PIN              8
    #define TX_PIN              9
    #define DEBUG_SERIAL        Serial
#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)
    #define RESET_PIN           7
    #define MODULE_SERIAL       Serial1
    #define DEBUG_SERIAL        Serial
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

#if defined(ARDUINO_ARCH_ESP8266)
    #define RESET_PIN           14
    #define RX_PIN              5
    #define TX_PIN              4
    #define DEBUG_SERIAL        Serial
#endif // ARDUINO_ARCH_ESP8266

#if defined(ARDUINO_ARCH_ESP32)
    #define RESET_PIN           14
    #define RX_PIN              12
    #define TX_PIN              13
    #define DEBUG_SERIAL        Serial
#endif // ARDUINO_ARCH_ESP32

//------------------------------------------------------------------------------
// Wize configuration
//------------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------


#if defined(MODULE_SERIAL)
    AllWize allwize(&MODULE_SERIAL, RESET_PIN);
#else
    AllWize allwize(RX_PIN, TX_PIN, RESET_PIN);
#endif

char buffer[300];

// -----------------------------------------------------------------------------
// Wize
// -----------------------------------------------------------------------------

void wizeSetup() {

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.println("# Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize.master();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "# Listening... CH %d, DR %d\n", allwize.getChannel(), allwize.getDataRate());
    DEBUG_SERIAL.print(buffer);
}

void wizeLoop() {

    if (allwize.available()) {

        // Get the message
        allwize_message_t message = allwize.read();

        if (CI_WIZE == message.ci) {
            snprintf(
                buffer, sizeof(buffer),
                "%02X%02X%02X%02X,%d,%d,",
                message.address[0], message.address[1],
                message.address[2], message.address[3],
                message.wize_counter, (int16_t) message.rssi / -2
            );
        } else {
            snprintf(
                buffer, sizeof(buffer),
                "%02X%02X%02X%02X,%d,%d,",
                message.address[0], message.address[1],
                message.address[2], message.address[3],
                message.c, (int16_t) message.rssi / -2
            );
        }

        DEBUG_SERIAL.print(buffer);

        for (uint8_t i=0; i<message.len; i++) {
            snprintf(buffer, sizeof(buffer), "%02X", message.data[i]);
            DEBUG_SERIAL.print(buffer);
        }
        DEBUG_SERIAL.println();

    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Setup serial DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println("# Wize 2 Serial bridge");

    wizeSetup();

}

void loop() {
    wizeLoop();
}
