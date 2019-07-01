/*

AllWize K1 + Arduino Zero Board

Simple slave that sends an auto-increment number every 5 seconds.

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

// -----------------------------------------------------------------------------
// Board definitions
// -----------------------------------------------------------------------------

#if not defined(ARDUINO_ARCH_SAMD)
    #error "This example is meant to run on an Arduino SAMD board!"
#endif

#define RESET_PIN           7
#define MODULE_SERIAL       Serial1
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

/*
#define RX_PIN              (10ul)
#define TX_PIN              (11ul)
#define SERCOM_PORT         sercom1
#define SERCOM_HANDLER      SERCOM1_Handler
#define SERCOM_MODE         PIO_SERCOM
#define SERCOM_RX_PAD       SERCOM_RX_PAD_2
#define SERCOM_TX_PAD       UART_TX_PAD_0
Uart SerialWize(&SERCOM_PORT, RX_PIN, TX_PIN, SERCOM_RX_PAD, SERCOM_TX_PAD);
void SERCOM_HANDLER() { SerialWize.IrqHandler(); }
#define MODULE_SERIAL       SerialWize
*/

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps
#define WIZE_UID                0x20212223

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

AllWize * allwize;

void wizeSetup() {

    DEBUG_SERIAL.println("Initializing radio module");

    #if defined(RX_PIN) && defined(TX_PIN)
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
        DEBUG_SERIAL.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize->slave();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);
    allwize->setUID(WIZE_UID);

    allwize->dump(DEBUG_SERIAL);

    DEBUG_SERIAL.println("[WIZE] Ready...");

}

void wizeSend(const char * payload) {

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "[WIZE] Sending '%s'\n", payload);
    DEBUG_SERIAL.print(buffer);

    if (!allwize->send(payload)) {
        DEBUG_SERIAL.println("[WIZE] Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Init serial DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[MAIN] Basic slave example");

    // Init radio
    wizeSetup();

}

void loop() {

    // This static variables will hold the number as int and char string
    static uint8_t count = 0;
    static char payload[4];

    // Convert the number to a string
    itoa(count, payload, 10);

    // Send the string as payload
    wizeSend(payload);

    // Increment the number (it will overflow at 255)
    count++;

    // Polling responses for 5 seconds
   delay(5000);

}
