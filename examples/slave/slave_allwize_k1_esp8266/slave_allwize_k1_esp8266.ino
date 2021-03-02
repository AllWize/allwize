/*

AllWize K1 + ESP8266-based Board

Simple slave that sends an auto-increment number every 5 seconds.

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

#if not defined(ARDUINO_ARCH_ESP8266)
    #error "This example is meant to run on an ESP8266 board!"
#endif

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
#define RX_PIN                  5
#define TX_PIN                  4
#define DEBUG_SERIAL            Serial

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps
#define WIZE_UID                0x20212223

// -----------------------------------------------------------------------------
// Global
// -----------------------------------------------------------------------------

AllWize allwize(RX_PIN, TX_PIN, RESET_PIN);

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

void wizeSetup() {

    DEBUG_SERIAL.println("Initializing radio module");

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true) delay(1);
    }

    allwize.slave();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);
    allwize.setUID(WIZE_UID);

    allwize.dump(DEBUG_SERIAL);

    DEBUG_SERIAL.println("[WIZE] Ready...");

}

void wizeSend(const char * payload) {

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "[WIZE] Sending '%s'\n", payload);
    DEBUG_SERIAL.print(buffer);

    if (!allwize.send(payload)) {
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

    // Delay responses for 20 seconds
    delay(20000);

}
