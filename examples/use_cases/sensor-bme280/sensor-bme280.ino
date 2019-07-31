/*

AllWize - BME280 Slave Example

Shows how to use aBME280 sensor (temperature, humidity & pressure)
to send environmental data.

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
    
    // Generic board
    /*
    #define RESET_PIN           7
    #define MODULE_SERIAL     Serial1
    #define DEBUG_SERIAL        SerialUSB
    */

    // AllWize K2
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
    #define MODULE_SERIAL     SerialWize
    #define RESET_PIN           (30u)
    #define DEBUG_SERIAL        SerialUSB

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

#define WIZE_CHANNEL        CHANNEL_04
#define WIZE_POWER          POWER_20dBm
#define WIZE_DATARATE       DATARATE_2400bps
#define WIZE_UID            0x20212223

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 sensor;

// -----------------------------------------------------------------------------
// I2C utils
// -----------------------------------------------------------------------------

#include <Wire.h>

bool i2cCheck(uint8_t address) {
    Wire.beginTransmission(address);
    return Wire.endTransmission();
}

void i2cScan() {

    Wire.begin();

    char buffer[64];
    uint8_t count = 0;

    for (uint8_t address = 1; address < 127; address++) {
        uint8_t error = i2cCheck(address);
        if (error == 0) {
            snprintf(
                buffer, sizeof(buffer),
                "[I2C] Device found at address 0x%02X",
                address
            );
            DEBUG_SERIAL.println(buffer);
            count++;
        }
    }

    snprintf(buffer, sizeof(buffer), "[I2C] %u device found", count);
    DEBUG_SERIAL.println(buffer);

}

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

#include "AllWize.h"
AllWize * allwize;

void wizeSetup() {

    DEBUG_SERIAL.println("Initializing radio module");

    #if defined(ARDUINO_ARCH_SAMD) && defined(RX_PIN) && defined(TX_PIN)
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
    allwize->setMode(MBUS_MODE_OSP);
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);
    allwize->setUID(WIZE_UID);

    DEBUG_SERIAL.println("[WIZE] Ready...");

}


void wizeSend(const char * payload) {

    DEBUG_SERIAL.print("[AllWize] Payload: ");
    DEBUG_SERIAL.println(payload);

    if (!allwize->send(payload)) {
        DEBUG_SERIAL.println("[AllWize] Error sending message");
    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Init DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[AllWize] BME280 sensor example");

    // I2C
    i2cScan();

    // Init BME280 sensor
    if (!sensor.begin(0x76)) {
        DEBUG_SERIAL.println("[AllWize] Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    DEBUG_SERIAL.println("[AllWize] Sensor ready!");

    // Init radio
    wizeSetup();

}

void loop() {

    double t_n = sensor.readTemperature();
    unsigned char h_n = sensor.readHumidity();
    unsigned long p_n = sensor.readPressure();

    char t_s[7];
    snfloat(t_s, sizeof(t_s), 1, t_n);

    char payload[20];
    snprintf(payload, sizeof(payload), "%s,%u,%lu", t_s, h_n, p_n);

    wizeSend(payload);

    delay(5000);

}
