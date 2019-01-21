/*

AllWize - LoRaWAN Packet Forwarder

Listens to messages on the same channel, data rate and CF and
forwards them to a LoRaWAN 1.X server using
Semtech legacy packet format

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

#if not defined(ARDUINO_ARCH_ESP8266)
    #error "This example is meant to run on an ESP8266 board!"
#endif

#include "configuration.h"
#include "debug.h"
#include "wifi.h"
#include "wize.h"
#include "ntp.h"
#include "lorawan.h"

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Setup debug
    debugSetup();

    // Init radio
    wizeSetup();

    // Setup NTP client
    ntpSetup();

    // Init LoRaWAN
    lorawanSetup();

    // Connect to wifi
    wifiSetup();
    wifiConnect();

}

void loop() {

    // Listen to messages
    wizeLoop();

    // Keep NTP time up to date
    ntpLoop();

    yield();

}
