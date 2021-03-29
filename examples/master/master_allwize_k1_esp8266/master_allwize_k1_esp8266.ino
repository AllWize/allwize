/*

AllWize K1 + ESP8266

Listens to messages on the same channel and data rate
and prints them out via the serial monitor.

Copyright (C) 2018-2021 by AllWize <github@allwize.io>

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

/*
----------------------------------------------------------
Wemos D1 R2 v2.1 pins:
----------------------------------------------------------
K1      Wemos   Function                        GPIO    S 
----------------------------------------------------------
RX<-0   RX 	    RXD 	                        RXD     *
TX->1   TX 	    TXD 	                        TXD     *
2       D0 	    I/O 	                        GPIO16  *
3       D1 	    I/O, SCL 	                    GPIO5   *
4       D2 	    I/O, SDA 	                    GPIO4   *
5       D3 	    I/O, 10k pull-up 	            GPIO0
6       D4 	    I/O, 10k pull-up, BUILTIN_LED 	GPIO2
7       D5 	    I/O, SCK 	                    GPIO14
8       D6 	    I/O, MISO 	                    GPIO12
9       D7 	    I/O, MOSI 	                    GPIO13
10      D8 	    I/O, 10k pull-down, SS 	        GPIO15  *
11      D7      I/O, MOSI 	                    GPIO13  *
12      D6      I/O, MISO 	                    GPIO12
13      D5      I/O, SCK 	                    GPIO14
A0      A0 	    Analog input 	                A0
A1              Analog input
A2              Analog input
A3              Analog input
A4              Analog input
A5              Analog input
GND     GND 	Ground 	                        GND
5V      5V 	    5V 	                            
3.3V    3V3 	3.3V 	                        3.3V
RESET   RST 	Reset 	                        RST
----------------------------------------------------------
*/

#define RESET_PIN               14
#define RX_PIN                  4
#define TX_PIN                  5
#define DEBUG_SERIAL            Serial

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_DATARATE           DATARATE_2400bps

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

AllWize allwize(RX_PIN, TX_PIN, RESET_PIN);

// -----------------------------------------------------------------------------
// Wize
// -----------------------------------------------------------------------------

void wizeSetup() {

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true) delay(1);
    }

    allwize.master();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setDataRate(WIZE_DATARATE);
    //allwize.softReset();

    allwize.dump(DEBUG_SERIAL);

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "[WIZE] Listening... CH %d, DR %d\n", allwize.getChannel(), allwize.getDataRate());
    DEBUG_SERIAL.print(buffer);

}

void wizeDebugMessage(allwize_message_t message) {

    // Code to pretty-print the message
    char buffer[256];
    if (CI_WIZE == message.ci) {
        snprintf(
            buffer, sizeof(buffer),
            "[WIZE] C: 0x%02X, CI: 0x%02X, MAN: %s, ADDR: 0x%02X%02X%02X%02X, VERSION: %d, KEY: %d, NETID: %d, APPID: %d, COUNTER: %d, RSSI: %d, DATA: 0x",
            message.c, message.ci, 
            message.man,
            message.address[0], message.address[1],
            message.address[2], message.address[3],
            message.wize_version, message.wize_key, message.wize_network_id, message.wize_application, message.wize_counter,
            (int16_t) message.rssi / -2
        );
    } else {
        snprintf(
            buffer, sizeof(buffer),
            "[WIZE] C: 0x%02X, CI: 0x%02X, MAN: %s, ADDR: 0x%02X%02X%02X%02X, RSSI: %d, DATA: 0x",
            message.c, message.ci, 
            message.man,
            message.address[0], message.address[1],
            message.address[2], message.address[3],
            (int16_t) message.rssi / -2
        );
    }
    DEBUG_SERIAL.print(buffer);

    for (uint8_t i=0; i<message.len; i++) {
        snprintf(buffer, sizeof(buffer), "%02X", message.data[i]);
        DEBUG_SERIAL.print(buffer);
    }
    DEBUG_SERIAL.println();

}

void wizeLoop() {

    if (allwize.available()) {

        // Get the message
        allwize_message_t message = allwize.read();

        // Show it to console
        wizeDebugMessage(message);

    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Setup serial DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (millis() < 2000) yield();
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[MAIN] Wize Master Example");
    DEBUG_SERIAL.print("[MAIN] Core version: ");
    DEBUG_SERIAL.println(ESP.getCoreVersion());
    DEBUG_SERIAL.println();

    // Init radio
    wizeSetup();

}

void loop() {

    // Listen to messages
    wizeLoop();

}
