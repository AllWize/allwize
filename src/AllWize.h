/*

AllWize Library

Copyright (C) 2018-2020 by AllWize <github@allwize.io>

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
 * @file AllWize.h
 * AllWize library header file
 */

#ifndef ALLWIZE_H
#define ALLWIZE_H

#include <Arduino.h>
#include "RC1701HP.h"
#include "OMS.h"
#include <Stream.h>
#if not defined(ARDUINO_ARCH_SAMD) && not defined(ARDUINO_ARCH_ESP32)
#include <SoftwareSerial.h>
#endif

// -----------------------------------------------------------------------------
// Types & definitions
// -----------------------------------------------------------------------------

// General
#define MODEM_DEFAULT_BAUDRATE          BAUDRATE_19200
#define GPIO_NONE                       0x99
#define RX_BUFFER_SIZE                  255
#define DEFAULT_TIMEOUT                 100
#define HARDWARE_SERIAL_PORT            1
#define DEFAULT_MBUS_MODE               MBUS_MODE_N1

#ifndef USE_MEMORY_CACHE
#define USE_MEMORY_CACHE                1
#endif

typedef struct {
    uint8_t c;
    uint8_t ci;
    char man[4];
    uint8_t type;
    uint8_t version;
    uint8_t address[4];
    uint8_t len;
    uint8_t data[RX_BUFFER_SIZE];
    uint8_t rssi;
    uint8_t wize_control;
    uint8_t wize_network_id;
    uint16_t wize_counter;
    uint8_t wize_application;
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

class AllWize {

    public:

        AllWize(HardwareSerial * serial, uint8_t reset_gpio = GPIO_NONE, uint8_t config_gpio = GPIO_NONE);
        #if not defined(ARDUINO_ARCH_SAMD) && not defined(ARDUINO_ARCH_ESP32)
        AllWize(SoftwareSerial * serial, uint8_t reset_gpio = GPIO_NONE, uint8_t config_gpio = GPIO_NONE);
        #endif
        AllWize(uint8_t rx, uint8_t tx, uint8_t reset_gpio = GPIO_NONE, uint8_t config_gpio = GPIO_NONE);

        void begin(uint8_t baudrate = MODEM_DEFAULT_BAUDRATE);
        bool reset();
        void softReset();
        bool factoryReset();
        void sleep();
        void wakeup();
        bool ready();
        bool waitForReady(uint32_t timeout = DEFAULT_TIMEOUT);
        void dump(Stream & debug);

        bool ack();
        bool send(uint8_t * buffer, uint8_t len);
        bool send(const char * buffer);
        bool available();
        bool enableRX(bool enable);
        allwize_message_t read();
        uint8_t * getBuffer();
        uint8_t getLength();

        void setControlInformation(uint8_t ci);
        uint8_t getControlInformation();

        void master();
        void slave();
        void repeater();

        void setChannel(uint8_t channel, bool persist = false);
        void setPower(uint8_t power, bool persist = false);
        void setDataRate(uint8_t dr);
        void setMode(uint8_t mode, bool persist = false);
        void setSleepMode(uint8_t mode);
        void setAppendRSSI(bool value);
        void setPreamble(uint8_t preamble);
        void setTimeout(uint16_t ms);
        void setNetworkRole(uint8_t role);
        void setLEDControl(uint8_t value);
        void setDataInterface(uint8_t value);
        void setControlField(uint8_t value, bool persist = false);
        void setInstallMode(uint8_t mode, bool persist = false);
        void setMAC2CheckOnlyFlag(uint8_t flag);
        void setEncryptFlag(uint8_t flag);
        void setDecryptFlag(uint8_t flag);
        void setKey(uint8_t reg, const uint8_t * key);
        void setDefaultKey(const uint8_t * key);
        void setAccessNumber(uint8_t value);
        void setBaudRate(uint8_t baudrate);

        uint8_t getChannel();
        uint8_t getPower();
        uint8_t getDataRate();
        uint8_t getMode();
        uint8_t getSleepMode();
        uint8_t getPreamble();
        uint8_t getDataInterface();
        uint8_t getControlField();
        bool getAppendRSSI();
        uint16_t getTimeout();
        uint8_t getNetworkRole();
        uint8_t getLEDControl();
        uint8_t getInstallMode();
        uint8_t getMAC2CheckOnlyFlag();
        uint8_t getEncryptFlag();
        uint8_t getDecryptFlag();
        void getDefaultKey(uint8_t * key);
        uint8_t getBaudRate();
        uint32_t getBaudRateSpeed(uint8_t value);

        float getRSSI();
        uint8_t getTemperature();
        uint16_t getVoltage();
        String getMID();
        bool setMID(uint16_t mid);
        String getUID();
        bool setUID(uint32_t uid);
        uint8_t getVersion();
        void setVersion(uint8_t version);
        uint8_t getDevice();
        void setDevice(uint8_t type);
        String getPartNumber();
        String getRequiredHardwareVersion();
        String getFirmwareVersion();
        String getSerialNumber();
        double getFrequency(uint8_t channel);
        uint16_t getDataRateSpeed(uint8_t dr);
        uint8_t getModuleType();
        String getModuleTypeName();

        // Wize specific
        bool setWizeControl(uint8_t wize_control);
        void setWizeOperatorId(uint8_t wize_network_id);
        void setWizeNetworkId(uint8_t wize_network_id);
        void setWizeApplication(uint8_t wize_application);
        void setCounter(uint16_t counter);
        uint16_t getCounter();

    protected:

        void _init();

        uint8_t _getAddress(uint8_t slot);
        bool _setConfig(bool value);
        int8_t _sendCommand(uint8_t command, uint8_t * data, uint8_t len);
        int8_t _sendCommand(uint8_t command, uint8_t data);
        int8_t _sendCommand(uint8_t command);

        bool _cacheMemory(uint8_t * buffer);
        uint8_t _getMemory(uint8_t address);
        uint8_t _getMemory(uint8_t address, uint8_t *buffer, uint8_t len);
        bool _setMemory(uint8_t address, uint8_t data);
        bool _setMemory(uint8_t address, uint8_t * data, uint8_t len);

        bool _setSlot(uint8_t slot, uint8_t data);
        bool _setSlot(uint8_t slot, uint8_t * data, uint8_t len);
        uint8_t _getSlot(uint8_t slot);
        uint8_t _getSlot(uint8_t slot, uint8_t * buffer, uint8_t len);
        String _getSlotAsHexString(uint8_t slot, uint8_t len);
        String _getSlotAsString(uint8_t slot, uint8_t len);

        void _readModel();
        bool _decode();

        void _flush();
        void _resetSerial();
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

        void _niceDelay(uint32_t ms);

    private:

    // -------------------------------------------------------------------------

    protected:

        int8_t _rx = -1;
        int8_t _tx = -1;

        Stream * _stream = NULL;
        HardwareSerial * _hw_serial = NULL;
        #if defined(ARDUINO_ARCH_SAMD)
            // Uart * _sw_serial = NULL;
        #elif defined(ARDUINO_ARCH_ESP32)
            // Nothing
        #else
            SoftwareSerial * _sw_serial = NULL;
        #endif

        uint8_t _reset_gpio = GPIO_NONE;
        uint8_t _config_gpio = GPIO_NONE;
        bool _config = false;
        uint32_t _timeout = DEFAULT_TIMEOUT;
        uint32_t _baudrate = 19200;
        
        uint8_t _ci = CI_APP_RESPONSE_UP_SHORT;
        uint8_t _mbus_mode = 0xFF;
        uint8_t _data_interface = 0xFF;
        bool _append_rssi = false;
        uint8_t _access_number = 0;
        uint8_t _module = MODULE_UNKNOWN;

        // Memory buffer
        #if USE_MEMORY_CACHE
            bool _ready = false;
            uint8_t _memory[0x100] = {0xFF};
        #endif

        String _model;
        String _hw;
        String _fw;

        // Wize specific
        uint8_t _wize_control = 0x40;
        uint16_t _wize_network_id = 0;
        uint8_t _wize_application = 0xFE;
        uint16_t _counter = 0;

        // Message buffers
        allwize_message_t _message;
        uint8_t _buffer[RX_BUFFER_SIZE];
        uint8_t _pointer = 0;
        uint8_t _length = 0;

};

#endif // ALLWIZE_H
