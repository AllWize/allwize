/*

AllWize - MCP9701 Thermistor Slave Example

Sends the data for the built-in MCP9701 thermistor in the AllWize K1.

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
    #define RESET_PIN           7
    #define MODULE_SERIAL       Serial1
    #define DEBUG_SERIAL        SerialUSB
#endif // ARDUINO_ARCH_SAMD

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_11
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps
#define WIZE_UID                0x20212223

#define TEMPERATURE_PIN         A2
#define TEMPERATURE_SAMPLES     10

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#if defined(MODULE_SERIAL)
    AllWize allwize(&MODULE_SERIAL, RESET_PIN);
#else
    AllWize allwize(RX_PIN, TX_PIN, RESET_PIN);
#endif

// -----------------------------------------------------------------------------
// Formatting
// -----------------------------------------------------------------------------

char * snfloat(char * buffer, uint8_t len, uint8_t decimals, float value) {

    bool negative = value < 0;
    if (negative) value = -value;

    uint32_t mul = 1;
    for (uint8_t i=0; i<decimals; i++) mul *= 10;

    uint32_t value_int = int(value);
    uint32_t value_dec = int(mul * (value - value_int));

    char format[20];
    snprintf(format, sizeof(format), "%s%%lu.%%0%ulu", negative ? "-" : "", decimals);
    snprintf(buffer, len, format, value_int, value_dec);

    return buffer;

}

// -----------------------------------------------------------------------------
// AllWize
// -----------------------------------------------------------------------------

void wizeSetup() {

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize.slave();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);
    allwize.setUID(WIZE_UID);

}

void wizeSend(const char * payload) {

    DEBUG_SERIAL.print("[AllWize] Payload: ");
    DEBUG_SERIAL.println(payload);

    if (!allwize.send(payload)) {
        DEBUG_SERIAL.println("[AllWize] Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Sensor reading
// -----------------------------------------------------------------------------

#if defined(ARDUINO_AVR_UNO)
    #define ANALOG_DEPTH        10
    #define ANALOG_REFERENCE    5000
#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)
    #define ANALOG_DEPTH        10
    #define ANALOG_REFERENCE    5000
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define ANALOG_DEPTH        12
    #define ANALOG_REFERENCE    3300
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)
    #define ANALOG_DEPTH        10
    #define ANALOG_REFERENCE    3300
#endif // ARDUINO_ARCH_ESP8266

#define ANALOG_COUNT            (1 << ANALOG_DEPTH)

// Get the temperature from the MCP9701 sensor on board attached to A2
// As per datasheet (page 8):
// The change in  voltage  is  scaled  to  a  temperature  coefficient  of
// 10.0 mV/°C   (typical)   for   the   MCP9700/9700A   and
// 19.5 mV/°C  (typical)  for  the  MCP9701/9701A.  The
// output voltage at 0°C is also scaled to 500 mV (typical)
// and  400 mV  (typical)  for  the  MCP9700/9700A  and
// MCP9701/9701A,  respectively.

double getTemperature() {
    double sum = 0;
    for (uint8_t i=0; i<TEMPERATURE_SAMPLES; i++) {
        sum += analogRead(TEMPERATURE_PIN);
    }
    double mV = sum * ANALOG_REFERENCE / ANALOG_COUNT / TEMPERATURE_SAMPLES;
    double t = (mV - 400.0) / 19.5;
    return t;
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Init serial DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[AllWize] MCP9701 sensor example");

    // Init temperature pin
    pinMode(TEMPERATURE_PIN, INPUT);
    #ifdef ARDUINO_ARCH_SAMD
        analogReadResolution(ANALOG_DEPTH);
    #endif

    // Init radio
    wizeSetup();

}

void loop() {

    float t = getTemperature();
    char payload[7];
    snfloat(payload, sizeof(payload), 2, t);

    wizeSend(payload);

    delay(5000);

}
