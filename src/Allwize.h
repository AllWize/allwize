/*

Allwize 0.0.1

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
#include <Stream.h>

#define MODEM_BAUDRATE          19200
#define END_OF_RESPONSE         '>'
#define CMD_ENTER_CONFIG        (char) 0x00
#define CMD_EXIT_CONFIG         (char) 0x58
#define CMD_EXIT_MEMORY         (char) 0xFF
#define RX_BUFFER_SIZE          32

#define CMD_CHANNEL             'C'
#define CMD_CONTROL_FIELD       'F'
#define CMD_MBUS_MODE           'G'
#define CMD_INSTALL             'I'
#define CMD_WRITE_MEMORY        'M'
#define CMD_AUTO_MESSAGE_FLAGS  'O'
#define CMD_OUTPUT_POWER        'P'
#define CMD_QUALITY             'Q'
#define CMD_READ_MAILBOX        'R'
#define CMD_RSSI                'S'
#define CMD_TEMPERATURE         'U'
#define CMD_VOLTAGE             'V'
#define CMD_READ_MEMORY         'Y'
#define CMD_SLEEP               'Z'

#define MEM_CHANNEL             0x00
#define MEM_RF_POWER            0x01
#define MEM_MBUS_MODE           0x03
#define MEM_SLEEP_MODE          0x04
#define MEM_RSSI_MODE           0x05
#define MEM_PREAMBLE_LENGTH     0x0A
#define MEM_TIMEOUT             0x10
#define MEM_NETWORK_ROLE        0x12
#define MEM_MAILBOX             0x16
#define MEM_MANUFACTURER_ID     0x19
#define MEM_UNIQUE_ID           0x1B
#define MEM_VERSION             0x1F
#define MEM_DEVICE              0x20
#define MEM_UART_BAUD_RATE      0x30
#define MEM_UART_FLOW_CTRL      0x35
#define MEM_DATA_INTERFACE      0x36
#define MEM_CONFIG_INTERFACE    0x37
#define MEM_FREQ_CAL            0x39
#define MEM_LED_CONTROL         0x3A
#define MEM_CONTROL_FIELD       0x3B
#define MEM_RX_TIMEOUT          0x3C
#define MEM_INSTALL_MODE        0x3D
#define MEM_ENCRYPT_FLAG        0x3E
#define MEM_DECRYPT_FLAG        0x3F
#define MEM_PART_NUMBER         0x61
#define MEM_HW_REV_NUMBER       0x6E
#define MEM_FW_REV_NUMBER       0x73
#define MEM_SERIAL_NUMBER       0x78

// MBus modes
typedef enum {
    MBUS_MODE_S = 0,
    MBUS_MODE_T1 = 1,
    MBUS_MODE_T2 = 2,
    MBUS_MODE_R2 = 4,
    MBUS_MODE_C1_T1 = 10,
    MBUS_MODE_C1_T2 = 11
} allwize_mbus_mode_t;

// Operation modes
typedef enum {
    INSTALL_MODE_NORMAL = 0,
    INSTALL_MODE_INSTALL = 1,
    INSTALL_MODE_HOST = 2
} allwize_install_mode_t;

class Allwize {

    public:

        Allwize(Stream& stream, uint8_t reset_gpio = 0xFF);

        void begin();
        void reset();
        void sleep();
        void wakeup();
        //void setTimeout(uint32_t timeout);

        void setMaster(bool master = true);

        void setChannel(uint8_t channel, bool persist = false);
        void setControlField(uint8_t value, bool persist = false);
        void setMBusMode(allwize_mbus_mode_t mode, bool persist = false);
        void setInstallMode(allwize_install_mode_t mode);

        uint8_t getChannel();
        uint8_t getControlField();
        allwize_mbus_mode_t getMBusMode();

        float getRSSI();
        uint8_t getTemperature();
        uint16_t getVoltage();

    protected:

        bool _setConfig(bool value);
        size_t _sendCommand(uint8_t command, uint8_t * data, size_t len);
        size_t _sendCommand(uint8_t command, uint8_t data);
        size_t _sendCommand(uint8_t command);
        void _setMemory(uint8_t address, uint8_t * data, size_t len);
        void _setMemory(uint8_t address, uint8_t data);
        size_t _getMemory(uint8_t address, size_t len, uint8_t * buffer);
        uint8_t _getMemory(uint8_t address);

        void _flush();
        size_t _send(uint8_t * buffer, size_t len);
        size_t _send(uint8_t ch);
        size_t _sendWait(uint8_t * buffer, size_t len);
        size_t _sendWait(uint8_t ch);
        size_t _receive();

        void _hex2bin(char * hex, uint8_t * bin, size_t len);
        void _bin2hex(uint8_t * bin, char * hex, size_t len);

    private:

    // -------------------------------------------------------------------------

    private:

        Stream& _stream;
        uint8_t _reset_gpio = 0xFF;
        bool _config = false;

        uint8_t _buffer[RX_BUFFER_SIZE];

};

#endif // ALLWIZE_H
