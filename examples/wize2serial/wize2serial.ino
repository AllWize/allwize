/*

AllWize - WIZE 2 Serial Bridge

Listens to messages on the same channel, data rate and CF and
forwards them via serial to a listening service on a connected computer.

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
// Board definitions
// -----------------------------------------------------------------------------

#if defined(ARDUINO_AVR_UNO)
    #define RESET_PIN           7
    #define RX_PIN              8
    #define TX_PIN              9
    #define PC_SERIAL           Serial
#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)
    #define RESET_PIN           7
    #define HARDWARE_SERIAL     Serial1
    #define PC_SERIAL           Serial
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define RESET_PIN           7
    #define HARDWARE_SERIAL     Serial1
    #define PC_SERIAL           SerialUSB
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)
    #define RESET_PIN           14
    #define RX_PIN              12
    #define TX_PIN              13
    #define PC_SERIAL           Serial
#endif // ARDUINO_ARCH_ESP8266

#if defined(ARDUINO_ARCH_ESP32)
    #define RESET_PIN           14
    #define RX_PIN              12
    #define TX_PIN              13
    #define PC_SERIAL           Serial
#endif // ARDUINO_ARCH_ESP32

//------------------------------------------------------------------------------
// Wize configuration
//------------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps

// -----------------------------------------------------------------------------
// Wize
// -----------------------------------------------------------------------------

#include "AllWize.h"
AllWize * allwize;
char buffer[256];

void wizeSetup() {

    // Create and init AllWize object
    #if defined(HARDWARE_SERIAL)
        allwize = new AllWize(&HARDWARE_SERIAL, RESET_PIN);
    #else
        allwize = new AllWize(RX_PIN, TX_PIN, RESET_PIN);
    #endif
    allwize->begin();
    if (!allwize->waitForReady()) {
        PC_SERIAL.println("# Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize->master();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);

    PC_SERIAL.println("# Listening...");

}

void wizeLoop() {

    if (allwize->available()) {

        // Get the message
        allwize_message_t message = allwize->read();

        // Send it to serial port
        snprintf(
            buffer, sizeof(buffer),
            "%d,%02X%02X%02X%02X,%d,%d,%d,%s\n",
            message.c,
            message.address[0], message.address[1],
            message.address[2], message.address[3],
            message.type, message.ci, message.rssi,
            (char *) message.data
        );
        PC_SERIAL.print(buffer);

    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Setup serial PC_SERIAL
    PC_SERIAL.begin(115200);
    while (!PC_SERIAL && millis() < 5000);
    PC_SERIAL.println("# Wize 2 Serial bridge");

    wizeSetup();

}

void loop() {
    wizeLoop();
}
