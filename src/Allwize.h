/*

AllWize Library

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

/**
 * @file Allwize library header file
 */

#ifndef ALLWIZE_H
#define ALLWIZE_H

#include <Arduino.h>
#include "Wize.h"
#include <Stream.h>
#if not defined(ARDUINO_ARCH_SAMD) && not defined(ARDUINO_ARCH_ESP32)
#include <SoftwareSerial.h>
#endif

// -----------------------------------------------------------------------------
// Types & definitions
// -----------------------------------------------------------------------------

// General
#define MODEM_BAUDRATE                  19200
#define GPIO_NONE                       0x99
#define CONTROL_INFORMATION             0x7A
#define END_OF_RESPONSE                 '>'
#define CMD_ENTER_CONFIG                (char) 0x00
#define CMD_EXIT_CONFIG                 (char) 0x58
#define CMD_EXIT_MEMORY                 (char) 0xFF
#define RX_BUFFER_SIZE                  255
#define DEFAULT_TIMEOUT                 1000
#define HARDWARE_SERIAL_PORT            1

typedef struct {
    uint8_t c;
    uint8_t ci;
    uint8_t len;
    uint8_t data[RX_BUFFER_SIZE];
    uint8_t rssi;
} allwize_message_t;

// -----------------------------------------------------------------------------
// DEBUG
// -----------------------------------------------------------------------------

// Uncomment this to the proper port
// or define it in your build settings if you want
// to get low level debug information via serial
//#define ALLWIZE_DEBUG_PORT Serial

#if defined(ALLWIZE_DEBUG_PORT)
    #define ALLWIZE_DEBUG_PRINT(...) ALLWIZE_DEBUG_PORT.print(__VA_ARGS__)
    #define ALLWIZE_DEBUG_PRINTLN(...) ALLWIZE_DEBUG_PORT.println(__VA_ARGS__)
#else
    #define ALLWIZE_DEBUG_PRINT(...)
    #define ALLWIZE_DEBUG_PRINTLN(...)
#endif

// -----------------------------------------------------------------------------
// Class prototype
// -----------------------------------------------------------------------------

class Allwize {

    public:

        Allwize(HardwareSerial * serial, uint8_t reset_gpio = GPIO_NONE);
        Allwize(uint8_t rx, uint8_t tx, uint8_t reset_gpio = GPIO_NONE);

        void begin();
        bool reset();
        bool factoryReset();
        void sleep();
        void wakeup();
        bool ready();
        bool waitForReady(uint32_t timeout = DEFAULT_TIMEOUT);
        void dump(Stream & debug);

        bool send(uint8_t * buffer, uint8_t len);
        bool send(const char * buffer);
        bool available();
        allwize_message_t read();

        void setControlInformation(uint8_t ci);
        uint8_t getControlInformation();

        void master();
        void slave();
        void repeater();

        void setChannel(uint8_t channel, bool persist = false);
        void setPower(uint8_t power, bool persist = false);
        void setDataRate(uint8_t dr);
        void setMBusMode(uint8_t mode, bool persist = false);
        void setSleepMode(uint8_t mode);
        //void setAppendRSSI(bool value);
        void setPreamble(uint8_t preamble);
        void setTimeout(uint8_t timeout);
        void setNetworkRole(uint8_t role);
        void setLEDControl(uint8_t value);
        //void setDataInterface(uint8_t value);
        void setControlField(uint8_t value, bool persist = false);
        void setInstallMode(uint8_t mode, bool persist = false);
        void setEncryptFlag(uint8_t flag);
        void setDecryptFlag(uint8_t flag);
        void setDefaultKey(uint8_t * key);

        uint8_t getChannel();
        uint8_t getPower();
        uint8_t getDataRate();
        uint8_t getMBusMode();
        uint8_t getSleepMode();
        uint8_t getPreamble();
        //uint8_t getDataInterface();
        uint8_t getControlField();
        //bool getAppendRSSI();
        uint8_t getTimeout();
        uint8_t getNetworkRole();
        uint8_t getLEDControl();
        uint8_t getInstallMode();
        uint8_t getEncryptFlag();
        uint8_t getDecryptFlag();
        void getDefaultKey(uint8_t * key);

        //float getRSSI();
        uint8_t getTemperature();
        uint16_t getVoltage();
        String getMID();
        String getUID();
        uint8_t getVersion();
        uint8_t getDevice();
        String getPartNumber();
        String getHardwareVersion();
        String getFirmwareVersion();
        String getSerialNumber();

    protected:

        bool _setConfig(bool value);
        int8_t _sendCommand(uint8_t command, uint8_t * data, uint8_t len);
        int8_t _sendCommand(uint8_t command, uint8_t data);
        int8_t _sendCommand(uint8_t command);
        bool _setMemory(uint8_t address, uint8_t * data, uint8_t len);
        bool _setMemory(uint8_t address, uint8_t data);
        uint8_t _getMemory(uint8_t address, uint8_t * buffer, uint8_t len);
        uint8_t _getMemory(uint8_t address);
        String _getMemoryAsHexString(uint8_t address, uint8_t len);
        String _getMemoryAsString(uint8_t address, uint8_t len);
        void _readModel();

        bool _decode();

        void _flush();
        void _reset_serial();
        uint8_t _send(uint8_t * buffer, uint8_t len);
        uint8_t _send(uint8_t ch);
        int8_t _receive();
        int8_t _sendAndReceive(uint8_t * buffer, uint8_t len);
        int8_t _sendAndReceive(uint8_t ch);

        int _timedRead();
        int _readBytes(char * buffer, uint16_t len);
        int _readBytesUntil(char terminator, char * buffer, uint16_t len);
        void _hex2bin(char * hex, uint8_t * bin, uint8_t len);
        void _bin2hex(uint8_t * bin, char * hex, uint8_t len);

    private:

    // -------------------------------------------------------------------------

    protected:

        int8_t _rx = -1;
        int8_t _tx = -1;

        Stream * _stream = NULL;
        HardwareSerial * _hw_serial = NULL;
        #if defined(ARDUINO_ARCH_SAMD)
            Uart * _sw_serial = NULL;
        #elif defined(ARDUINO_ARCH_ESP32)
            // Nothing
        #else
            SoftwareSerial * _sw_serial = NULL;
        #endif

        uint8_t _reset_gpio = GPIO_NONE;
        bool _config = false;
        uint32_t _timeout = DEFAULT_TIMEOUT;
        uint8_t _ci = CONTROL_INFORMATION;

        String _model;
        String _fw;
        String _hw;

        allwize_message_t _message;

        uint8_t _buffer[RX_BUFFER_SIZE];
        uint8_t _pointer;

};

#endif // ALLWIZE_H
