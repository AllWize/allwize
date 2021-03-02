/*

AllWize - Simple Slave Example - AllWize K2

Simple slave that sends faked data using MBUS frame format.

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
#include "MBUSPayload.h"

// -----------------------------------------------------------------------------
// Board configuration
// -----------------------------------------------------------------------------

// Check http://wiki.allwize.io/index.php?title=Allwize_K2#Arduino_IDE
// to add support to Allwize K2 board in Arduino IDE
#if not defined(ARDUINO_ALLWIZE_K2)
    #error "This example is meant to run on an AllWize K2 board!"
#endif

#define MODULE_SERIAL           SerialWize
#define DEBUG_SERIAL            SerialUSB
#define RESET_PIN               PIN_WIZE_RESET

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps
#define WIZE_UID                0x20212223

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

AllWize allwize(&MODULE_SERIAL, RESET_PIN);
MBUSPayload payload(32);

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

void wizeSetup() {

    DEBUG_SERIAL.println("Initializing radio module");

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize.slave();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);
    allwize.setUID(WIZE_UID);

    allwize.dump(DEBUG_SERIAL);

    DEBUG_SERIAL.println("[WIZE] Ready...");

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
// Dummy sensors
// -----------------------------------------------------------------------------

void sensorSetup() {
    randomSeed(analogRead(A0));
}

// Returns the temperature in C (with 1 decimal)
float getTemperature() {
    return (float) random(100, 300) / 10.0;
}

// Returns the himidity in %
float getHumidity() {
    return (float) random(30, 90);
}

// Returns the pressure in hPa (with 1 decimal)
float getPressure() {
    return (float) random(9800, 10300) / 10.0;
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

    // This static variables will hold the frame counter 
    static uint16_t count = 0;

    // Build payload
    payload.reset();
    payload.addField(MBUS_CODE_ACCESS_NUMBER, count++);
    payload.addField(MBUS_CODE_FLOW_TEMPERATURE_C, getTemperature());
    payload.addField(MBUS_CODE_PRESSURE_BAR, -3, getPressure()); // 1hPa == 1mbar

    // Send the string as payload
    wizeSend(payload.getBuffer(), payload.getSize());

    // Delay responses for 20 seconds
    delay(20000);

}
