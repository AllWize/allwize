/*

Allwize - MCP9701 Thermistor Slave Example

This example sends the data for the built-in MCP9701 thermistor in the AllWize K1.

Copyright (C) 2018 by Allwize <github@allwize.io>

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
    #define RX_PIN      8
    #define TX_PIN      9
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
    #define debug       Serial
#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)
    #define module      Serial1
    #define debug       Serial
#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)
    #define module      Serial1
    #define debug       SerialUSB
#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)
    #define RX_PIN      12
    #define TX_PIN      13
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
    #define debug       Serial
#endif // ARDUINO_ARCH_ESP8266

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#define WIZE_CHANNEL            0x04
#define WIZE_DATARATE           0x01
#define WIZE_NETWORK_ID         0x46
#define WIZE_NODE_ID            0x08

#define TEMPERATURE_PIN         A2
#define TEMPERATURE_SAMPLES     10

// -----------------------------------------------------------------------------
// Formatting
// -----------------------------------------------------------------------------

char * snfloat(char * buffer, size_t len, size_t decimals, float value) {

    bool negative = value < 0;

    uint32_t mul = 1;
    for (uint8_t i=0; i<decimals; i++) mul *= 10;

    value = abs(value);
    uint32_t value_int = int(value);
    uint32_t value_dec = int((value - value_int) * mul);

    char format[20];
    snprintf(format, sizeof(format), "%s%%lu.%%0%ulu", negative ? "-" : "", decimals);
    snprintf(buffer, len, format, value_int, value_dec);

    return buffer;

}

// -----------------------------------------------------------------------------
// Allwize
// -----------------------------------------------------------------------------

#include "Allwize.h"
Allwize * allwize;

void wizeSetup() {

    module.begin(19200);
    allwize = new Allwize(module);
    allwize->begin();
    while (!allwize->ready());

    allwize->slave();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(5);
    allwize->setDataRate(WIZE_DATARATE);
    allwize->setControlField(WIZE_NETWORK_ID);
    allwize->setControlInformation(WIZE_NODE_ID);

}

void wizeSend(const char * payload) {

    debug.print("[Allwize] Payload: ");
    debug.println(payload);

    if (!allwize->send(payload)) {
        debug.println("[Allwize] Error sending message");
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

    // Init serial debug
    debug.begin(115200);
    while (!debug && millis() < 5000);
    debug.println();
    debug.println("[Allwize] MCP9701 sensor example");

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
