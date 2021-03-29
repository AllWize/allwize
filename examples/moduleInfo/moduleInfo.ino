/*

AllWize - Module Info Example

Pretty-prints out the configuration settings stored in the module non-volatile memory.

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

#endif // ARDUINO_AVR_LEONARDO

#if defined(ARDUINO_ARCH_SAMD)

    // Common:
    #define DEBUG_SERIAL        SerialUSB

    #if defined(ARDUINO_ALLWIZE_K2)
        
        #define RESET_PIN       PIN_WIZE_RESET
        #define MODULE_SERIAL   SerialWize

    #else
        
        // Arduino M0 / M0Pro
        #define RESET_PIN       7
        #define MODULE_SERIAL   Serial1

        // Custom configuration combinations:
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
        #define RESET_PIN           (30ul)
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
        #define MODULE_SERIAL       SerialWize
        */

    #endif

#endif // ARDUINO_ARCH_SAMD

#if defined(ARDUINO_ARCH_ESP8266)

    // Common:
    #define RESET_PIN           14
    #define DEBUG_SERIAL        Serial

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

    // Using Software serial:
    #define RX_PIN              5
    #define TX_PIN              4

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
// Globals
// -----------------------------------------------------------------------------

#if defined(MODULE_SERIAL)
    AllWize allwize(&MODULE_SERIAL, RESET_PIN);
#else
    AllWize allwize(RX_PIN, TX_PIN, RESET_PIN);
#endif

// -----------------------------------------------------------------------------
// Utils
// -----------------------------------------------------------------------------

#define COLUMN_PAD  25

void format(const char * name, const char * value, bool isHex = false) {
    DEBUG_SERIAL.print(name);
    for (uint8_t i=0; i<COLUMN_PAD-strlen(name); i++) DEBUG_SERIAL.print(" ");
    if (isHex) DEBUG_SERIAL.print("0x");
    DEBUG_SERIAL.println(value);
}

void format(const char * name, String value, bool isHex = false) {
    DEBUG_SERIAL.print(name);
    for (uint8_t i=0; i<COLUMN_PAD-strlen(name); i++) DEBUG_SERIAL.print(" ");
    if (isHex) DEBUG_SERIAL.print("0x");
    DEBUG_SERIAL.println(value);
}

void format(const char * name, int value, bool asHex = false) {
    char buffer[10];
    if (asHex) {
        snprintf(buffer, sizeof(buffer), "0x%X", value);
    } else {
        snprintf(buffer, sizeof(buffer), "%d", value);
    }
    format(name, buffer);
}

// -----------------------------------------------------------------------------
// Radio
// -----------------------------------------------------------------------------

unsigned char wizeAutoBaud() {

    char baudrates[] = { 
        BAUDRATE_19200, BAUDRATE_2400, BAUDRATE_4800, BAUDRATE_9600, 
        BAUDRATE_14400, BAUDRATE_28800, BAUDRATE_38400, BAUDRATE_57600, 
        BAUDRATE_76800, BAUDRATE_115200, BAUDRATE_230400 };

    for(unsigned char i=0; i<sizeof(baudrates);i++) {
        
        unsigned char baudrate = baudrates[i];
        DEBUG_SERIAL.print("Checking baudrate ");
        DEBUG_SERIAL.println(allwize.getBaudRateSpeed(baudrate));
        
        allwize.begin(baudrate);
        if (allwize.waitForReady()) {
            return baudrate;
        }

    }

    return 0;


}

void wizeSetup() {

    DEBUG_SERIAL.println("Initializing radio module");

    #if defined(ARDUINO_ARCH_SAMD) && defined(RX_PIN) && defined(TX_PIN)
        pinPeripheral(RX_PIN, SERCOM_MODE);
        pinPeripheral(TX_PIN, SERCOM_MODE);
    #endif

    // Init AllWize object
    unsigned char baudrate = wizeAutoBaud();
    if (baudrate == 0) {
        DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        while (true) delay(1);
    }

    DEBUG_SERIAL.print("Radio module OK");

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    DEBUG_SERIAL.begin(115200);
    #if defined(ARDUINO_ARCH_ESP8266) or defined(ARDUINO_ARCH_ESP32)
        delay(5000);
    #else
        while (!DEBUG_SERIAL && millis() < 5000);
    #endif
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

    format("Module type", allwize.getModuleTypeName());
    format("UART speed", allwize.getBaudRateSpeed(allwize.getBaudRate()));
    format("Channel", allwize.getChannel());
    format("Power", allwize.getPower());
    format("MBUS Mode", allwize.getMode(), true);
    format("Sleep Mode", allwize.getSleepMode());
    format("Data Rate", allwize.getDataRateSpeed(allwize.getDataRate()));
    format("Preamble Length", allwize.getPreamble());
    format("Control Field", allwize.getControlField(), true);
    format("Network Role", allwize.getNetworkRole());
    format("Install Mode", allwize.getInstallMode());

    format("Manufacturer ID", allwize.getMID(), true);
    format("Unique ID", allwize.getUID(), true);
    format("Device Type", allwize.getDeviceType());
    format("Device Version", allwize.getDeviceVersion());
    format("Part Number", allwize.getPartNumber());
    format("Firmware Version", allwize.getFirmwareVersion());
    format("Req. Hardware Version", allwize.getRequiredHardwareVersion());
    format("Serial Number", allwize.getSerialNumber(), true);

    format("Temperature (C)", allwize.getTemperature());
    format("Voltage (mV)", allwize.getVoltage());

    // -------------------------------------------------------------------------

    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("Memory dump:");

    allwize.dump(DEBUG_SERIAL);

    DEBUG_SERIAL.println("Done");


}

void loop() {
    delay(1);
}
