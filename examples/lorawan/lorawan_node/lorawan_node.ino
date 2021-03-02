/*

AllWize - LoRaWAN Node example

Simple slave that sends dummy data every 20s.
The example uses CayenneLPP library to encode the data.
You can enable the Cayenne LPP payload format decoder in your application server to get decoded data.
If using TTN it is under your application "Payload Formats" tab.
If using LoRaServer configure it in your Device Profile.

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

#include "AllWize_LoRaWAN.h"
#include "CayenneLPP.h"

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
    
    #if defined(ARDUINO_ALLWIZE_K2)
        #define RESET_PIN           PIN_WIZE_RESET
        #define MODULE_SERIAL       SerialWize
        #define DEBUG_SERIAL        SerialUSB
    #else
        #define RESET_PIN           7
        #define MODULE_SERIAL       Serial1
        #define DEBUG_SERIAL        SerialUSB
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

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps

#define LORAWAN_PORT            1

uint8_t DEVADDR[4] = { 0x26, 0x01, 0x13, 0x3D };
uint8_t NWKSKEY[16] = { 0xF6, 0x59, 0x13, 0x16, 0x99, 0x35, 0x0E, 0xBF, 0x41, 0xE5, 0xEA, 0xA6, 0x95, 0xFE, 0x64, 0x27 };
uint8_t APPSKEY[16] = { 0xA2, 0x40, 0xE9, 0xF6, 0xB1, 0xB1, 0x3B, 0x53, 0xB1, 0xE2, 0x0A, 0x0D, 0x3D, 0x50, 0x74, 0xCB };

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#if defined(MODULE_SERIAL)
    AllWize_LoRaWAN allwize(&MODULE_SERIAL, RESET_PIN);
#else
    AllWize_LoRaWAN allwize(RX_PIN, TX_PIN, RESET_PIN);
#endif
CayenneLPP lpp(16);

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
// AllWize
// -----------------------------------------------------------------------------

void wizeSetup() {

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true);
    }

    // WIZE radio settings
    allwize.slave();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);

    // LoRaWan settings
    allwize.joinABP(DEVADDR, APPSKEY, NWKSKEY);

    DEBUG_SERIAL.println();
    DEBUG_SERIAL.print("Module Type     : "); DEBUG_SERIAL.println(allwize.getModuleTypeName());
    DEBUG_SERIAL.print("Unique ID       : "); DEBUG_SERIAL.println(allwize.getUID());
    DEBUG_SERIAL.print("Firmware Version: "); DEBUG_SERIAL.println(allwize.getFirmwareVersion());
    DEBUG_SERIAL.print("Serial Number   : "); DEBUG_SERIAL.println(allwize.getSerialNumber());
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[WIZE] Ready...");
    DEBUG_SERIAL.println();

}

void wizeSend(uint8_t * payload, size_t len) {

    char buffer[64];
    snprintf(buffer, sizeof(buffer),
        "[WIZE] CH: %d, TX: %d, DR: %d, Payload: ",
        allwize.getChannel(), allwize.getPower(), allwize.getDataRate()
    );
    DEBUG_SERIAL.print(buffer);

    for (uint8_t i = 0; i<len; i++) {
        snprintf(buffer, sizeof(buffer), "%02X", payload[i]);
        DEBUG_SERIAL.print(buffer);
    }
    DEBUG_SERIAL.print("\n");

    if (!allwize.send(payload, len, LORAWAN_PORT)) {
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
    DEBUG_SERIAL.println("[MAIN] LoRaWAN node");

    // Init radio
    wizeSetup();

    // Init (dummy) sensor
    sensorSetup();

}

void loop() {

    // Payload
    lpp.reset();
    lpp.addTemperature(1, getTemperature());
    lpp.addRelativeHumidity(2, getHumidity());
    lpp.addBarometricPressure(3, getPressure());

    // Send the payload
    wizeSend(lpp.getBuffer(), lpp.getSize());

    // Wait 20 seconds and redo
    delay(20000);

}
