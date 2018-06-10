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

// -----------------------------------------------------------------------------
// Types & definitions
// -----------------------------------------------------------------------------

// General
#define MODEM_BAUDRATE                  19200
#define CONTROL_INFORMATION             0x7A
#define END_OF_RESPONSE                 '>'
#define CMD_ENTER_CONFIG                (char) 0x00
#define CMD_EXIT_CONFIG                 (char) 0x58
#define CMD_EXIT_MEMORY                 (char) 0xFF
#define RX_BUFFER_SIZE                  255
#define DEFAULT_TIMEOUT                 1000

// Command keys
#define CMD_CHANNEL                     'C'
#define CMD_CONTROL_FIELD               'F'
#define CMD_MBUS_MODE                   'G'
#define CMD_INSTALL_MODE                'I'
#define CMD_WRITE_MEMORY                'M'
#define CMD_RF_POWER                    'P'
#define CMD_QUALITY                     'Q'
#define CMD_RSSI                        'S'
#define CMD_TEMPERATURE                 'U'
#define CMD_VOLTAGE                     'V'
#define CMD_READ_MEMORY                 'Y'
#define CMD_SLEEP                       'Z'

// Memory addresses
#define MEM_CHANNEL                     0x00
#define MEM_RF_POWER                    0x01
#define MEM_DATA_RATE                   0x02
#define MEM_MBUS_MODE                   0x03
#define MEM_SLEEP_MODE                  0x04
#define MEM_RSSI_MODE                   0x05
#define MEM_PREAMBLE_LENGTH             0x0A
#define MEM_TIMEOUT                     0x10
#define MEM_NETWORK_ROLE                0x12
#define MEM_MAILBOX                     0x16
#define MEM_MANUFACTURER_ID             0x19
#define MEM_UNIQUE_ID                   0x1B
#define MEM_VERSION                     0x1F
#define MEM_DEVICE                      0x20
#define MEM_UART_BAUD_RATE              0x30
#define MEM_UART_FLOW_CTRL              0x35
#define MEM_DATA_INTERFACE              0x36
#define MEM_CONFIG_INTERFACE            0x37
#define MEM_FREQ_CAL                    0x39
#define MEM_LED_CONTROL                 0x3A
#define MEM_CONTROL_FIELD               0x3B
#define MEM_RX_TIMEOUT                  0x3C
#define MEM_INSTALL_MODE                0x3D
#define MEM_ENCRYPT_FLAG                0x3E
#define MEM_DECRYPT_FLAG                0x3F
#define MEM_DEFAULT_KEY                 0x40
#define MEM_PART_NUMBER_OLD             0x61
#define MEM_SERIAL_NUMBER_OLD           0x71
#define MEM_PART_NUMBER_NEW             0x89
#define MEM_SERIAL_NUMBER_NEW           0xA9

// Channels
#define CHANNEL_01                      0x01
#define CHANNEL_02                      0x02
#define CHANNEL_03                      0x03
#define CHANNEL_04                      0x04
#define CHANNEL_05                      0x05
#define CHANNEL_06                      0x06
#define CHANNEL_07                      0x07
#define CHANNEL_08                      0x08
#define CHANNEL_09                      0x09
#define CHANNEL_10                      0x10
#define CHANNEL_11                      0x11
#define CHANNEL_12                      0x12
#define CHANNEL_13                      0x13
#define CHANNEL_14                      0x14
#define CHANNEL_15                      0x15
#define CHANNEL_16                      0x16
#define CHANNEL_17                      0x17
#define CHANNEL_18                      0x18
#define CHANNEL_19                      0x19
#define CHANNEL_20                      0x20
#define CHANNEL_21                      0x21
#define CHANNEL_22                      0x22
#define CHANNEL_23                      0x23
#define CHANNEL_24                      0x24
#define CHANNEL_25                      0x25
#define CHANNEL_26                      0x26
#define CHANNEL_27                      0x27
#define CHANNEL_28                      0x28
#define CHANNEL_29                      0x29
#define CHANNEL_30                      0x30
#define CHANNEL_31                      0x31
#define CHANNEL_32                      0x32
#define CHANNEL_33                      0x33
#define CHANNEL_34                      0x34
#define CHANNEL_35                      0x35
#define CHANNEL_36                      0x36
#define CHANNEL_37                      0x37
#define CHANNEL_38                      0x38
#define CHANNEL_39                      0x39
#define CHANNEL_40                      0x40
#define CHANNEL_41                      0x41

// Data rates
#define DATARATE_2400bps                0x01
#define DATARATE_4800bps                0x02
#define DATARATE_19200bps               0x04
#define DATARATE_6400bps                0x05

// Power modes
#define POWER_10dBm                     0x01
#define POWER_14dBm                     0x02
#define POWER_17dBm                     0x03
#define POWER_20dBm                     0x04
#define POWER_24dBm                     0x05

// MBus modes
#define MBUS_MODE_S2                    0x00
#define MBUS_MODE_T1                    0x01
#define MBUS_MODE_T2                    0x02
#define MBUS_MODE_S1                    0x03
#define MBUS_MODE_R                     0x04
#define MBUS_MODE_T1_C                  0x0A
#define MBUS_MODE_T2_C                  0x0B
#define MBUS_MODE_N2                    0x10
#define MBUS_MODE_N1                    0x11
#define MBUS_MODE_OSP                   0x12

// Operation modes
#define INSTALL_MODE_NORMAL             0x00
#define INSTALL_MODE_INSTALL            0x01
#define INSTALL_MODE_HOST               0x02

// Sleep modes
#define SLEEP_MODE_DISABLE              0x00
#define SLEEP_MODE_AFTER_TX             0x01
#define SLEEP_MODE_AFTER_TX_RX          0x03
#define SLEEP_MODE_AFTER_TX_TIMEOUT     0x05
#define SLEEP_MODE_AFTER_TX_RX_TIMEOUT  0x07

// Network roles
#define NETWORK_ROLE_SLAVE              0x00
#define NETWORK_ROLE_MASTER             0x01
#define NETWORK_ROLE_REPEATER           0x02

// Timeouts
#define TIMEOUT_32MS                    0x01
#define TIMEOUT_48MS                    0x02
#define TIMEOUT_64MS                    0x03
#define TIMEOUT_2S                      0x7C
#define TIMEOUT_4S                      0xF9

// LED Control
#define LED_CONTROL_DISABLED            0x00
#define LED_CONTROL_RX_TX               0x01
#define LED_CONTROL_UART_RF_IDLE        0x02

// Encrypt/Decrypt flags
#define ENCRYPT_DISABLED                0x00
#define ENCRYPT_ENABLED                 0x01
#define ENCRYPT_ENABLED_CRC             0x03

// Data interface
#define DATA_INTERFACE_ID_ADDR          0x00
#define DATA_INTERFACE_APP_ONLY         0x01
#define DATA_INTERFACE_APP_ACK          0x03
#define DATA_INTERFACE_START_STOP       0x04
#define DATA_INTERFACE_CRC              0x08
#define DATA_INTERFACE_CRC_START_STOP   0x0C


// Preamble Length
#define PREAMBLE_FORMAT_A               0x00
#define PREAMBLE_FORMAT_B               0x02

typedef struct {
    uint8_t c;
    uint8_t ci;
    uint8_t len;
    uint8_t * data;
    uint8_t rssi;
} allwize_message_t;

// -----------------------------------------------------------------------------
// Class prototype
// -----------------------------------------------------------------------------

class Allwize {

    public:

        Allwize(Stream & stream, uint8_t reset_gpio = 0xFF);

        void begin();
        bool reset();
        bool factoryReset();
        void sleep();
        void wakeup();
        bool ready();
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

        Stream & _stream;

        uint8_t _reset_gpio = 0xFF;
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
