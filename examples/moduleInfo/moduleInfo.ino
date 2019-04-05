/*

AllWize - Module Info Example

Pretty-prints out the configuration settings stored in the module non-volatile memory.

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
// Config & globals
// -----------------------------------------------------------------------------

#include "AllWize.h"
AllWize * allwize;

// -----------------------------------------------------------------------------
// Board definitions
// -----------------------------------------------------------------------------

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MOTEINO)

    // Common
    #define RESET_PIN           7
    #define DEBUG_SERIAL        Serial

    // Using software serial:
    #define RX_PIN              8
    #define TX_PIN              9

#endif // ARDUINO_AVR_UNO

#if defined(ARDUINO_AVR_LEONARDO)

    // Common:
    #define RESET_PIN           7
    #define DEBUG_SERIAL        Serial

    // Using hardware serial:
    #define MODULE_SERIAL       Serial1

    // Using Software serial:
    //#define RX_PIN              8
    //#define TX_PIN              9
    //SoftwareSerial SerialWize(RX_PIN, TX_PIN);
    //#define MODULE_SERIAL       SerialWize

#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)

    // Common:
    #define RESET_PIN           7
    #define DEBUG_SERIAL        SerialUSB

    // Using exposed hardware serials:
    #define MODULE_SERIAL       Serial1

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
    #define RX_PIN              10
    #define TX_PIN              11
    #define SERCOM_PORT         sercom3
    #define SERCOM_HANDLER      SERCOM3_Handler
    #define SERCOM_MODE         PIO_SERCOM_ALT
    #define SERCOM_RX_PAD       SERCOM_RX_PAD_2
    #define SERCOM_TX_PAD       UART_TX_PAD_0
    #include "wiring_private.h" // pinPeripheral() function
    Uart SerialWize(&SERCOM_PORT, RX_PIN, TX_PIN, SERCOM_RX_PAD, SERCOM_TX_PAD);
    void SERCOM_HANDLER() { SerialWize.IrqHandler(); }
    #define MODULE_SERIAL       SerialWize
    */

#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)

    // Common:
    #define RESET_PIN           14
    #define DEBUG_SERIAL        Serial

    // Using Software serial:
    #define RX_PIN              12
    #define TX_PIN              13

#endif // ARDUINO_ARCH_ESP8266

#if defined(ARDUINO_ARCH_ESP32)

    // Common:
    #define RESET_PIN           14
    #define DEBUG_SERIAL        Serial

    // Using Hardware serial on random GPIOs:
    #define RX_PIN              12
    #define TX_PIN              13

#endif // ARDUINO_ARCH_ESP32

// -----------------------------------------------------------------------------
// Utils
// -----------------------------------------------------------------------------

#define COLUMN_PAD  20

void format(const char * name, const char * value) {
    DEBUG_SERIAL.print(name);
    for (uint8_t i=0; i<COLUMN_PAD-strlen(name); i++) DEBUG_SERIAL.print(" ");
    DEBUG_SERIAL.println(value);
}

void format(const char * name, String value) {
    DEBUG_SERIAL.print(name);
    for (uint8_t i=0; i<COLUMN_PAD-strlen(name); i++) DEBUG_SERIAL.print(" ");
    DEBUG_SERIAL.println(value);
}

void format(const char * name, int value) {
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", value);
    format(name, buffer);
}

// -----------------------------------------------------------------------------
// Radio
// -----------------------------------------------------------------------------

void wizeSetup() {

    DEBUG_SERIAL.println("Checking radio module");

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
        DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        while (true);
    }

    DEBUG_SERIAL.println("Radio module OK");

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("AllWize - Module Info");
    DEBUG_SERIAL.println();

    // -------------------------------------------------------------------------

    wizeSetup();

    // -------------------------------------------------------------------------

    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("Module info:");
    DEBUG_SERIAL.println();

    format("Property", "Value");
    DEBUG_SERIAL.println("------------------------------");

    format("Channel", allwize->getChannel());
    format("Power", allwize->getPower());
    format("MBUS Mode", allwize->getMBusMode());
    format("Sleep Mode", allwize->getSleepMode());
    format("Data Rate", allwize->getDataRate());
    format("Preamble Length", allwize->getPreamble());
    format("Control Field", allwize->getControlField());
    format("Network Role", allwize->getNetworkRole());
    format("Install Mode", allwize->getInstallMode());

    format("Manufacturer ID", allwize->getMID());
    format("Unique ID", allwize->getUID());
    format("Version", allwize->getVersion());
    format("Device", allwize->getDevice());
    format("Part Number", allwize->getPartNumber());
    format("Hardware Version", allwize->getHardwareVersion());
    format("Firmware Version", allwize->getFirmwareVersion());
    format("Serial Number", allwize->getSerialNumber());

    format("Temperature (C)", allwize->getTemperature());
    format("Voltage (mV)", allwize->getVoltage());

    // -------------------------------------------------------------------------

    delay(1000);

    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("Memory dump:");

    allwize->dump(DEBUG_SERIAL);

    DEBUG_SERIAL.println("Done");


}

void loop() {}
