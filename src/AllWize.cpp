/*

AllWize Library

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

#include "AllWize.h"
#include <assert.h>

// -----------------------------------------------------------------------------
// Init
// -----------------------------------------------------------------------------

/**
 * @brief               AllWize object constructor
 * @param stream        HardwareSerial object to communicate with the module
 * @param _reset_gpio   GPIO connected to the module RESET pin
 */
AllWize::AllWize(HardwareSerial * serial, uint8_t reset_gpio) : _stream(serial), _hw_serial(serial), _reset_gpio(reset_gpio) {
    _init();
}

/**
 * @brief               AllWize object constructor
 * @param stream        SoftwareSerial object to communicate with the module
 * @param _reset_gpio   GPIO connected to the module RESET pin
 */
#if not defined(ARDUINO_ARCH_SAMD) && not defined(ARDUINO_ARCH_ESP32)
AllWize::AllWize(SoftwareSerial * serial, uint8_t reset_gpio) : _stream(serial), _sw_serial(serial), _reset_gpio(reset_gpio) {
    _init();
}
#endif

/**
 * @brief               AllWize object constructor
 * @param rx            GPIO for RX
 * @param tx            GPIO for TX
 * @param _reset_gpio   GPIO connected to the module RESET pin
 */
AllWize::AllWize(uint8_t rx, uint8_t tx, uint8_t reset_gpio) : _rx(rx), _tx(tx), _reset_gpio(reset_gpio) {
    #if defined(ARDUINO_ARCH_SAMD)
        // Software serial not implemented for SAMD
        assert(false);
    #elif defined(ARDUINO_ARCH_ESP32)
        _stream = _hw_serial = new HardwareSerial(HARDWARE_SERIAL_PORT);
    #else
        _stream = _sw_serial = new SoftwareSerial(_rx, _tx);
    #endif
    _init();
}

void AllWize::_init() {
    if (GPIO_NONE != _reset_gpio) {
        pinMode(_reset_gpio, OUTPUT);
        digitalWrite(_reset_gpio, HIGH);
    }
    randomSeed(analogRead(0));
    _access_number = random(0,256);
}

/**
 * @brief               Inits the module communications
 */
void AllWize::begin() {
    reset();
    delay(200);
    _append_rssi = _getMemory(MEM_RSSI_MODE) == 0x01;
    _mbus_mode = _getMemory(MEM_MBUS_MODE);
    _data_interface = _getMemory(MEM_DATA_INTERFACE);
}

/**
 * @brief               Resets the serial object
 */
void AllWize::_reset_serial() {

    if (_hw_serial) {

        _hw_serial->end();
        #if defined(ARDUINO_ARCH_ESP32)
            if ((_rx != -1) && (_tx != -1)) {
                pinMode(_rx, FUNCTION_4);
                pinMode(_tx, FUNCTION_4);
                _hw_serial->begin(MODEM_BAUDRATE, SERIAL_8N1, _rx, _tx);
            } else {
                _hw_serial->begin(MODEM_BAUDRATE);
            }
        #else
            _hw_serial->begin(MODEM_BAUDRATE);
        #endif

    } else {

        #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_SAMD)
            // It should never hit this block
            assert(false);
        #else
            _sw_serial->end();
            _sw_serial->begin(MODEM_BAUDRATE);
        #endif

    }

    _flush();

}

/**
 * @brief               Resets the radio module.
 * @return              Reset successfully issued
 */
bool AllWize::reset() {
    if (GPIO_NONE == _reset_gpio) {
        _reset_serial();
        delay(100);
        _setMemory(MEM_CONFIG_INTERFACE, 1);
        if (_setConfig(true)) {
            _send('@');
            _send('R');
            _send('R');
            _flush();
            delay(100);
            _config = false;
            _reset_serial();
            return true;
        }
    } else {
        digitalWrite(_reset_gpio, LOW);
        delay(1);
        digitalWrite(_reset_gpio, HIGH);
        delay(100);
        _config = false;
        _reset_serial();
        return true;
    }
    return false;
}

/**
 * @brief               Resets the module to factory settings
 * @return              Factory reset successfully issued
 */
bool AllWize::factoryReset() {
    _reset_serial();
    delay(100);
    _setMemory(MEM_CONFIG_INTERFACE, 1);
    if (_setConfig(true)) {
        _send('@');
        _send('R');
        _send('C');
        _flush();
        delay(100);
        _config = false;
        _reset_serial();
        return true;
    }
    return false;
}

/**
 * @brief               Sets the module in master mode
 */
void AllWize::master() {
    setMBusMode(DEFAULT_MBUS_MODE, true);
    setNetworkRole(NETWORK_ROLE_MASTER);
    setInstallMode(INSTALL_MODE_HOST);
    setSleepMode(SLEEP_MODE_DISABLE);
    setPower(POWER_20dBm);
    setDataRate(DATARATE_2400bps);
    setDataInterface(DATA_INTERFACE_START_STOP);
    setAppendRSSI(true);
}

/**
 * @brief               Sets the module in slave mode
 */
void AllWize::slave() {
    setMBusMode(DEFAULT_MBUS_MODE, true);
    setNetworkRole(NETWORK_ROLE_SLAVE);
    setPower(POWER_20dBm);
    setDataRate(DATARATE_2400bps);
}

/**
 * @brief               Sets the module in repeater mode
 */
void AllWize::repeater() {
    setMBusMode(DEFAULT_MBUS_MODE, true);
    setNetworkRole(NETWORK_ROLE_REPEATER);
}

/**
 * @brief               Sets the radio module in sleep mode
 */
void AllWize::sleep() {
    if (!_setConfig(true)) return;
    _send(CMD_SLEEP);
}

/**
 * @brief               Wakes up the radio from sleep mode
 */
void AllWize::wakeup() {
    _send(CMD_AWAKE);
    ready();
}

/**
 * @brief               Test whether the radio module is ready or not
 */
bool AllWize::ready() {
    bool response = _setConfig(true);
    if (response) _setConfig(false);
    return response;
}

/**
 * @brief               Waits for timeout millis for the module to be ready
 */
bool AllWize::waitForReady(uint32_t timeout) {
    uint32_t start = millis();
    while (millis() - start < timeout) {
        if (ready()) return true;
        delay(100);
    }
    return false;
}

/**
 * @brief               Dumps the current memory configuration to the given stream
 * @param debug         Data stream to dump the data to
 */
void AllWize::dump(Stream & debug) {

    _setConfig(true);
    _send(CMD_TEST_MODE_0);

    char buffer[256];
    if (256 == _readBytes(buffer, 256)) {

        char ch[10];
        char ascii[17] = {0};
        ascii[16] = 0;

        debug.println();
        debug.print("       ");
        for (uint16_t address = 0; address <= 0x0F; address++) {
            snprintf(ch, sizeof(ch), "%02X ", address);
            debug.print(ch);
        }
        debug.println();
        debug.print("------------------------------------------------------");

        for (uint16_t address = 0; address <= 255; address++) {
            if ((address % 16) == 0) {
                if (address > 0) debug.print(ascii);
                snprintf(ch, sizeof(ch), "\n0x%02X:  ", address);
                debug.print(ch);
            }
            if (31 < buffer[address] && buffer[address] < 127) {
                ascii[address % 16] = (char) buffer[address];
            } else {
                ascii[address % 16] = ' ';
            }
            snprintf(ch, sizeof(ch), "%02X ", (uint8_t) buffer[address]);
            debug.print(ch);
        }

        debug.println();
        debug.println();

    } else {
        debug.println("Error doing memory dump...");
    }

    _setConfig(false);

}

/**
 * @brief               Sends a byte array
 * @param buffer        Byte array with the application payload
 * @param len           Length of the payload
 * @return              Returns true if message has been correctly sent
 */
bool AllWize::send(uint8_t * buffer, uint8_t len) {

    if (_config) return false;
    if (0 == len) return (1 == _send(0xFE));

    // length
    if (1 != _send(len+1)) return false;

    // CI
    if (1 != _send(_ci)) return false;

    // payload
    if (len != _send(buffer, len)) return false;

    _access_number++;
    return true;

}

/**
 * @brief               Sends c-string
 * @param buffer        C-string with the application payload
 * @return              Returns true if message has been correctly sent
 */
bool AllWize::send(const char * buffer) {
    return send((uint8_t *) buffer, strlen(buffer));
}

/**
 * @brief               Returns true if a new message has been received and decoded
 *                      This method has to be called in the main loop to monitor for incomming messages
 * @return              Whether a new message is available
 */
bool AllWize::available() {

    bool response = false;

    if (!_config) {

        static uint32_t when = millis();
        
        while (_stream->available() && _pointer < RX_BUFFER_SIZE) {
            uint8_t ch = _stream->read();
            _buffer[_pointer++] = ch;
            when = millis();
            #if defined(ARDUINO_ARCH_ESP8266)
                yield();
            #endif
        }

        // Check if message finished and decode it
        if ((_pointer > 0) && (millis() - when > 100)) {
            response = _decode();
            _pointer = 0;
        }

    }

    return response;

}

/**
 * @brief               Returns latest received message
 * @return              New message
 */
allwize_message_t AllWize::read() {
    return _message;
}

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

/**
 * @brief               Sets the control information byte
 * @param  ci           CI byte value
 */
void AllWize::setControlInformation(uint8_t ci) {
    _ci = ci;
}

/**
 * @brief               Gets the control information byte
 * @return              CI byte value
 */
uint8_t AllWize::getControlInformation() {
    return _ci;
}

/**
 * @brief               Sets the communications channel (for MBUS_MODE_R2 only)
 * @param channel       Channel number
 * @param persist       Persist the changes in non-volatile memory (defaults to False)
 */
void AllWize::setChannel(uint8_t channel, bool persist) {
    if (persist) {
        _setMemory(MEM_CHANNEL, channel);
    }
    _sendCommand(CMD_CHANNEL, channel);
}

/**
 * @brief               Gets the channel stored in non-volatile memory
 * @return              Channel (1 byte)
 */
uint8_t AllWize::getChannel() {
    return _getMemory(MEM_CHANNEL);
}

/**
 * @brief               Sets the RF power
 * @param power         Value from 1 to 5
 * @param persist       Persist the changes in non-volatile memory (defaults to False)
 */
void AllWize::setPower(uint8_t power, bool persist) {
    if (0 < power && power < 6) {
        if (persist) {
            _setMemory(MEM_RF_POWER, power);
        }
        _sendCommand(CMD_RF_POWER, power);
    }
}

/**
 * @brief               Gets the RF power stored in non-volatile memory
 * @return              RF power (1 byte)
 */
uint8_t AllWize::getPower() {
    return _getMemory(MEM_RF_POWER);
}

/**
 * @brief               Sets the data rate
 * @param dr            Value in [1, 2, 4, 5]
 */
void AllWize::setDataRate(uint8_t dr) {
    if (0 < dr && dr < 6 && dr != 3) {
        _setMemory(MEM_DATA_RATE, dr);
    }
}

/**
 * @brief               Gets the data rate stored in non-volatile memory
 * @return              Current data rate (1 byte)
 */
uint8_t AllWize::getDataRate() {
    return _getMemory(MEM_DATA_RATE);
}

/**
 * @brief               Sets the module in one of the available MBus modes
 * @param mode          MBus mode (MBUS_MODE_*)
 * @param persist       Persist the changes in non-volatile memory (defaults to False)
 */
void AllWize::setMBusMode(uint8_t mode, bool persist) {
    if (persist) {
        _setMemory(MEM_MBUS_MODE, mode);
    }
    _sendCommand(CMD_MBUS_MODE, mode);
    _mbus_mode = mode;
}

/**
 * @brief               Gets the MBus mode stored in non-volatile memory
 * @return              MBus mode (1 byte)
 */
uint8_t AllWize::getMBusMode() {
    return _mbus_mode;
}

/**
 * @brief               Sets the sleep mode
 * @param mode          One of SLEEP_MODE_*
 */
void AllWize::setSleepMode(uint8_t mode) {
    _setMemory(MEM_SLEEP_MODE, mode);
}

/**
 * @brief               Gets the sleep mode stored in non-volatile memory
 * @return              Sleep mode (1 byte)
 */
uint8_t AllWize::getSleepMode() {
    return _getMemory(MEM_SLEEP_MODE);
}

/**
 * @brief               Sets the RSSI mode value
 * @param value         Set to true to append RSSI value to received data
 */
void AllWize::setAppendRSSI(bool value) {
    if (value == 1) {
        _setMemory(MEM_RSSI_MODE, 1);
    } else {
        _setMemory(MEM_RSSI_MODE, 0);
    }
    _append_rssi = value;
}

/**
 * @brief               Gets the current RSSI mode value
 * @return              True if RSSI value will be appended to received data
 */
bool AllWize::getAppendRSSI() {
    return _append_rssi;
}

/**
 * @brief               Sets the preamble length frame format
 * @param preamble      0 or 2
 */
void AllWize::setPreamble(uint8_t preamble) {
    if (PREAMBLE_FORMAT_A == preamble || PREAMBLE_FORMAT_B == preamble) {
        _setMemory(MEM_PREAMBLE_LENGTH, preamble);
    }
}

/**
 * @brief               Gets the preamble length frame format
 * @return              Preamble length format (1 byte)
 */
uint8_t AllWize::getPreamble() {
    return _getMemory(MEM_PREAMBLE_LENGTH);
}

/**
 * @brief               Sets the timeout for auto sleep modes
 * @param timeout       Timeout value (defaults to 1s)
 */
void AllWize::setTimeout(uint8_t timeout) {
    _setMemory(MEM_TIMEOUT, timeout);
}

/**
 * @brief               Gets the current timeout for auto sleep modes
 * @return              Timeout setting
 */
uint8_t AllWize::getTimeout() {
    return _getMemory(MEM_TIMEOUT);
}

/**
 * @brief               Sets the network role
 * @param role          Network role (NETWORK_ROLE_*)
 */
void AllWize::setNetworkRole(uint8_t role) {
    _setMemory(MEM_NETWORK_ROLE, role);
}

/**
 * @brief               Gets the current network role
 * @return              Network role
 */
uint8_t AllWize::getNetworkRole() {
    return _getMemory(MEM_NETWORK_ROLE);
}

/**
 * @brief               Sets the LED control
 * @param value         LED control value
 */
void AllWize::setLEDControl(uint8_t value) {
    _setMemory(MEM_LED_CONTROL, value);
}

/**
 * @brief               Gets the current LED control
 * @return              LED control value
 */
uint8_t AllWize::getLEDControl() {
    return _getMemory(MEM_LED_CONTROL);
}

/**
 * @brief               Sets the data interface for receiving packets
 * @param               Value from 0x00 to 0x0C
 */
void AllWize::setDataInterface(uint8_t value) {
    if (value <= 0x0C) {
        _setMemory(MEM_DATA_INTERFACE, value);
        _data_interface = value;
    }
}

/**
 * @brief               Gets the data interface for receiving packets
 * @return              Value (1 byte)
 */
uint8_t AllWize::getDataInterface() {
    return _data_interface;
}

/**
 * @brief               Sets the control field value
 * @param value         Control field
 * @param persist       Persist the changes in non-volatile memory (defaults to False)
 */
void AllWize::setControlField(uint8_t value, bool persist) {
    if (persist) {
        _setMemory(MEM_CONTROL_FIELD, value);
    }
    _sendCommand(CMD_CONTROL_FIELD, value);
}

/**
 * @brief               Gets the control field value stored in non-volatile memory
 * @return              Control field value (1 byte)
 */
uint8_t AllWize::getControlField() {
    return _getMemory(MEM_CONTROL_FIELD);
}

/**
 * @brief               Sets the module in one of the available operations modes
 * @param mode          Operation mode
 * @param persist       Persist the changes in non-volatile memory (defaults to False)
 */
void AllWize::setInstallMode(uint8_t mode, bool persist) {
    if (mode <= 2) {
        if (persist) {
            _setMemory(MEM_INSTALL_MODE, mode);
        }
        _sendCommand(CMD_INSTALL_MODE, mode);
    }
}

/**
 * @brief               Gets the install modevalue stored in non-volatile memory
 * @return              Install mode value (1 byte)
 */
uint8_t AllWize::getInstallMode() {
    return _getMemory(MEM_INSTALL_MODE);
}

/**
 * @brief               Sets the encrypt flag setting
 * @param flag          Encrypt flag
 */
void AllWize::setEncryptFlag(uint8_t flag) {
    if (0 == flag || 1 == flag || 3 == flag) {
        _encrypt = (flag & 0x01) == 0x01;
        _setMemory(MEM_ENCRYPT_FLAG, flag);
    }
}

/**
 * @brief               Gets the encrypt flag setting
 * @return              Encrypt flag
 */
uint8_t AllWize::getEncryptFlag() {
    return _getMemory(MEM_ENCRYPT_FLAG);
}

/**
 * @brief               Sets the decrypt flag setting
 * @param flag          Decrypt flag
 */
void AllWize::setDecryptFlag(uint8_t flag) {
    _setMemory(MEM_DECRYPT_FLAG, flag);
}

/**
 * @brief               Gets the decrypt flag setting
 * @return              Decrypt flag
 */
uint8_t AllWize::getDecryptFlag() {
    return _getMemory(MEM_DECRYPT_FLAG);
}

/**
 * @brief               Sets the default encryption key
 * @param reg           Register number (1-64)
 * @param key           A 16-byte encryption key as binary array
 */
void AllWize::setKey(uint8_t reg, const uint8_t * key) {
    if (0 < reg && reg < 65) {
        uint8_t data[17];
        data[0] = reg;
        memcpy(&data[1], key, 16);
        _sendCommand(CMD_KEY_REGISTER, data, 17);
    }
}

/**
 * @brief               Sets the default encryption key
 * @param key           A 16-byte encryption key as binary array
 */
void AllWize::setDefaultKey(const uint8_t * key) {
    _setMemory(MEM_DEFAULT_KEY, (uint8_t *) key, 16);
}

/**
 * @brief               Gets the default encryption key
 * @param key           A binary buffer to store the key (16 bytes)
 */
void AllWize::getDefaultKey(uint8_t * key) {
    _getMemory(MEM_DEFAULT_KEY, key, 16);
}

// -----------------------------------------------------------------------------

/**
 * @brief               Returns the RSSI of the last valid packet received
 *                      TODO: values do not seem right and are not the same as in the packet
 * @return              RSSI in dBm
 */
/*
float AllWize::getRSSI() {
    uint8_t response = _sendCommand(CMD_RSSI);
    if (response > 0) return -0.5 * _buffer[0];
    return 0;
}
*/

/**
 * @brief               Returns the internal temperature of the module
 * @return              Temperature in Celsius
 */
uint8_t AllWize::getTemperature() {
    uint8_t response = _sendCommand(CMD_TEMPERATURE);
    uint8_t ret_val = 0;

    if (response > 0) {
        ret_val = _buffer[0] - 128;
    } else {
        ret_val = 0;
    }
    
    return ret_val;
}

/**
 * @brief               Returns the internal voltage of the module
 * @return              Voltage in mV
 */
uint16_t AllWize::getVoltage() {
    uint8_t response = _sendCommand(CMD_VOLTAGE);
    uint16_t ret_val;
    if (response > 0) {
        ret_val = 30 * _buffer[0];
    } else {
        ret_val = 0;
    }
    
    return ret_val;
}

/**
 * @brief               Returns the Manufacturer ID string
 * @return              2-byte hex string with the manufacturer ID
 */
String AllWize::getMID() {
    return _getMemoryAsHexString(MEM_MANUFACTURER_ID, 2);
}

/**
 * @brief               Sets the Manufacturer ID
 * @uint16_t mid        MID to save
 */
bool AllWize::setMID(uint16_t mid) {
    uint8_t buffer[2];
    buffer[0] = (mid >> 8) & 0xFF;
    buffer[1] = (mid >> 0) & 0xFF;
    return _setMemory(MEM_MANUFACTURER_ID, buffer, 2);
}

/**
 * @brief               Returns the Unique ID string
 * @return              4-byte hex string with the unique ID
 */
String AllWize::getUID() {
    return _getMemoryAsHexString(MEM_UNIQUE_ID, 4);
}

/**
 * @brief               Saved the UID into the module memory
 * @uint32_t uid        UID to save
 */
bool AllWize::setUID(uint32_t uid) {
    uint8_t buffer[4];
    buffer[0] = (uid >> 24) & 0xFF;
    buffer[1] = (uid >> 16) & 0xFF;
    buffer[2] = (uid >>  8) & 0xFF;
    buffer[3] = (uid >>  0) & 0xFF;
    return _setMemory(MEM_UNIQUE_ID, buffer, 4);
}

/**
 * @brief               Returns the module version from non-volatile memory
 * @return              Version
 */
uint8_t AllWize::getVersion() {
    return _getMemory(MEM_VERSION);
}

/**
 * @brief               Returns the device version from non-volatile memory
 * @return              Device
 */
uint8_t AllWize::getDevice() {
    return _getMemory(MEM_DEVICE);
}

/**
 * @brief               Returns the module part number
 * @return              12-byte hex string with the part number
 */
String AllWize::getPartNumber() {
    _readModel();
    return _model;
}

/**
 * @brief               Returns the module hardware revision
 * @return              4-byte hex string with the HW version
 */
String AllWize::getHardwareVersion() {
    _readModel();
    return _hw;
}

/**
 * @brief               Returns the module firmware revision
 * @return              4-byte hex string with the FW version
 */
String AllWize::getFirmwareVersion() {
    _readModel();
    return _fw;
}

/**
 * @brief               Returns the module serial number
 * @return              8-byte hex string with the serial number
 */
String AllWize::getSerialNumber() {
    return _getMemoryAsHexString(MEM_SERIAL_NUMBER_NEW, 8);
}

// -----------------------------------------------------------------------------
// Protected
// -----------------------------------------------------------------------------

/**
 * @brief               Sets or unsets config mode
 * @param value         True to enter config mode
 * @return              True if in config mode
 * @protected
*/
bool AllWize::_setConfig(bool value) {
    if (value != _config) {
        _flush();
        if (value) {
            if (_sendAndReceive(CMD_ENTER_CONFIG) == 0) {
                _config = true;
                delay(2);
            }
        } else {
            _send(CMD_EXIT_CONFIG);
            _config = false;
            delay(10);
        }
    }
    return _config;
}

/**
 * @brief               Sends a command with the given data
 * @param command       Command key
 * @param data          Binary data to send
 * @param len           Length of the binary data
 * @protected
 */
int8_t AllWize::_sendCommand(uint8_t command, uint8_t * data, uint8_t len) {
    int8_t response = -1;
    if (!_setConfig(true)) return response;
    if (_sendAndReceive(command) != -1) {
        response = _sendAndReceive(data, len);
    }
    _setConfig(false);
    return response;
}

/**
 * @brief               Sends a command with the given data
 * @param command       Command key
 * @param data          Single byte
 * @return              Number of bytes received, -1 if timed out or error sending
 * @protected
 */
int8_t AllWize::_sendCommand(uint8_t command, uint8_t data) {
    int8_t response = -1;
    if (!_setConfig(true)) return response;
    if (_sendAndReceive(command) != -1) {
        response = _sendAndReceive(data);
    }
    _setConfig(false);
    return response;
}

/**
 * @brief               Sends a command with no data
 * @param  command      Command key
 * @return              Number of bytes received, -1 if timed out or error sending
 * @protected
 */
int8_t AllWize::_sendCommand(uint8_t command) {
    int8_t response = -1;
    if (!_setConfig(true)) return response;
    response = _sendAndReceive(command);
    _setConfig(false);
    return response;
}

/**
 * @brief               Sets non-volatile memory contents starting from given address
 * @param  address      Command key
 * @param data          Binary data to store
 * @param len           Length of the binary data
 * @return              True if the data was successfully saved
 * @protected
 */
bool AllWize::_setMemory(uint8_t address, uint8_t * data, uint8_t len) {
    uint8_t buffer[len*2+1];
    for (uint8_t i=0; i<len; i++) {
        buffer[i*2]   = address + i;
        buffer[i*2+1] = data[i];
    }
    buffer[len*2] = CMD_EXIT_MEMORY;
    return (_sendCommand(CMD_WRITE_MEMORY, buffer, len*2+1) != -1);
}

/**
 * @brief               Sets non-volatile memory contents starting from given address
 * @param address       Command key
 * @param data          Single byte to store at given address
 * @return              True if the data was successfully saved
 * @protected
 */
bool AllWize::_setMemory(uint8_t address, uint8_t value) {
    uint8_t buffer[3] = {address, value, (uint8_t) CMD_EXIT_MEMORY};
    return (_sendCommand(CMD_WRITE_MEMORY, buffer, 3) != -1);
}

/**
 * @brief               Returns the contents of consecutive memory addresses
 * @param address       Address to start from
 * @param buffer        Buffer with at least 'len' position to store data to
 * @param len           Number of positions to read
 * @return              Number of positions actually read
 * @protected
 */
uint8_t AllWize::_getMemory(uint8_t address, uint8_t * buffer, uint8_t len) {
    uint8_t count = 0;
    if (_setConfig(true)) {
        for (uint8_t i=0; i<len; i++) {
            if (_sendAndReceive(CMD_READ_MEMORY) == -1) break;
            if (_sendAndReceive(address + i) != 1) break;
            count++;
            buffer[i] = _buffer[0];
        }
        _setConfig(false);
    }
    return count;
}

/**
 * @brief               Returns the contents of single memory addresses
 * @param address       Address to start from
 * @return              Contents of the address, 0 if error
 * @protected
 */
uint8_t AllWize::_getMemory(uint8_t address) {
    uint8_t response = _sendCommand(CMD_READ_MEMORY, address);
    if (response > 0) return _buffer[0];
    return 0;
}

/**
 * @brief               Returns the contents of the memory from a certain address as an HEX String
 * @param address       Address to start from
 * @param len           Number of bytes to read
 * @return              Result (empty string if error)
 * @protected
 */
String AllWize::_getMemoryAsHexString(uint8_t address, uint8_t len) {
    uint8_t bin[len];
    char hex[2*len+1];
    hex[0] = 0;
    if (len == _getMemory(address, bin, len)) {
        _bin2hex(bin, hex, len);
    }
    return String(hex);
}

/**
 * @brief               Returns the contents of the memory from a certain address as a String object
 * @param address       Address to start from
 * @param len           Number of bytes to read
 * @return              Result (empty string if error)
 * @protected
 */
String AllWize::_getMemoryAsString(uint8_t address, uint8_t len) {
    uint8_t bin[len];
    char hex[len+1];
    hex[0] = 0;
    if (len == _getMemory(address, bin, len)) {
        memcpy(hex, bin, len);
        hex[len-1] = 0;
    }
    return String(hex);
}

/**
 * @brief               Reads and caches the module model & version
 */
void AllWize::_readModel() {

    if (_model.length() > 0) return;
    String line = _getMemoryAsString(MEM_PART_NUMBER_NEW, 32);
    if (line.indexOf("RC") != 0) String line = _getMemoryAsString(MEM_PART_NUMBER_OLD, 32);

    if (line.indexOf("RC") != 0) {
        _model = String("Unknown");

    } else {

        uint8_t start = 0;
        uint8_t end = line.indexOf(",", start);
        _model = line.substring(start, end);

        start = end + 1;
        end = line.indexOf(",", start);
        _hw = line.substring(start, end);

        start = end + 1;
        end = line.indexOf(",", start);
        _fw = line.substring(start, end);
        _fw.trim();

    }

}

/**
 * @brief               Decodes the current RX buffer contents
 * @return              Whether the contents are a valid packet
 *
 * Message format depending current configuration
 *
 *    Data interface           | 0 | 1 | 3 | 4 | 8 | C |
 * -----------------------------------------------------
 * 0. Start byte (0x68)        |   |   |   | * |   | * |
 * 1. Length (1-byte)          | * | * | * | * | * | * | length of sections 2 to 7
 * 2. Control field (1-byte)   | * |   |   | * | * | * |
 * 3. Header (8-bytes)         | * |   |   | * | * | * | manID (2 bytes) + address (6 bytes)
 * 4. Control info (1-byte)    | * | * | * | * | * | * |
 * 5. App data (n-bytes)       | * | * | * | * | * | * |
 * 6. RSSI (1-byte)            | - | - | - | - | - | - |
 * 7. CRC (2-bytes)            |   |   |   |   | * | * |
 * 8. Stop byte (0x16)         |   |   |   | * |   | * |
 *
 * Note:
 *  - Data Interface defaults to 0x04 for this version and RSSI enabled
 *  - Modes 0x01 and 0x03 not working in MBUS_MODE_OSP
 *  - Modes 0x01 and 0x03 should be the same, but 0x03 does ACK
 *  - Section 6 (RSSI) present always and only if RSSI_MODE == 1
 *  - Section 2 (C) and 3 (HEADER) not present in MBUS_MODE_OSP
 *
 */
bool AllWize::_decode() {

    #if defined(ALLWIZE_DEBUG_PORT)
    {
        char ch[4];
        ALLWIZE_DEBUG_PRINT("recv:");
        for (uint8_t i = 0; i < _pointer; i++) {
            snprintf(ch, sizeof(ch), " %02X", _buffer[i]);
            ALLWIZE_DEBUG_PRINT(ch);
        }
        ALLWIZE_DEBUG_PRINTLN();
    }
    #endif

    // Local copy of the buffer
    uint8_t _local[RX_BUFFER_SIZE];
    memcpy(_local, _buffer, RX_BUFFER_SIZE);

    // Get current values
    uint8_t mbus_mode = getMBusMode();
    uint8_t data_interface = getDataInterface();
    bool has_start = (data_interface & 0x04) == 0x04;
    bool has_header = (mbus_mode != MBUS_MODE_OSP) & ((data_interface & 0x01) == 0x00);
    bool has_rssi = getAppendRSSI();
    bool has_crc = (data_interface & 0x08) == 0x08;
    uint8_t bytes_not_in_len = has_start ? 3 : 1;
    uint8_t bytes_not_in_app = (has_header ? 9 : 0) + 1 + (has_rssi ? 1 : 0) + (has_crc ? 2 : 0);

    // This variable will contain the pointer to the current reading position
    uint8_t in = 0;

    // Start byte
    if (has_start) {
        if (START_BYTE != _local[in]) return false;
        in += 1;
    };

    // Get and check buffer length
    uint8_t len = _local[in];
    if (_pointer != len + bytes_not_in_len) return false;
    in += 1;

    if (has_header) {

        // C-field
        _message.c = _local[in];
        in += 1;

        // Manufacturer
        unsigned int man = (_local[in+1] << 8) + _local[in];
        _message.man[0] = ((man >> 10) & 0x001F) + 64;
        _message.man[1] = ((man >>  5) & 0x001F) + 64;
        _message.man[2] = ((man >>  0) & 0x001F) + 64;
        _message.man[3] = 0;
        in += 2;

        // Address
        _message.address[0] = _local[in + 3];
        _message.address[1] = _local[in + 2];
        _message.address[2] = _local[in + 1];
        _message.address[3] = _local[in + 0];

        // Type
        _message.type = _local[in + 5];

        // Version
        _message.version = _local[in + 4];

        in += 6;

    } else {
        _message.c = 0xFF;
        _message.type = 0;
        _message.version = 0;
        _message.man[0] = 0;
        memset(_message.address, 0, 6);
    }

    // Control information
    _message.ci = _buffer[in];
    in += 1;

    // Application data
    _message.len = len - bytes_not_in_app;
    memcpy(_message.data, &_local[in], _message.len);
    _message.data[_message.len] = 0;
    in += _message.len;

    // RSSI
    if (has_rssi) {
        _message.rssi = _local[in];
        in += 1;
    } else {
        _message.rssi = 0xFF;
    }

    // CRC
    if (has_crc) {
        in += 2;
    }

    // Stop byte
    if (has_start) {
        if (STOP_BYTE != _local[in]) return false;
    }

    return true;

}

// -----------------------------------------------------------------------------

/**
 * @brief               Flushes the serial line to the module
 * @protected
 */
void AllWize::_flush() {
    _stream->flush();
}

/**
 * @brief               Sends a single byte to the module UART. Returns the number of bytes actually sent.
 * @param ch            Byte to send
 * @return              Number of bytes actually sent
 * @protected
 */
uint8_t AllWize::_send(uint8_t ch) {
    #if defined(ALLWIZE_DEBUG_PORT)
    {
        char buffer[10];
        snprintf(buffer, sizeof(buffer), "w %02X '%c'", ch, (32 <= ch && ch <= 126) ? ch : 32);
        ALLWIZE_DEBUG_PRINTLN(buffer);
    }
    #endif
    return _stream->write(ch);
}

/**
 * @brief               Sends a binary buffer to the module UART. Returns the number of bytes actually sent.
 * @param buffer        Binary data to send
 * @param len           Length of the binary data
 * @return              Number of bytes actually sent
 * @protected
 */
uint8_t AllWize::_send(uint8_t * buffer, uint8_t len) {
    uint8_t n = 0;
    for (uint8_t i=0; i<len; i++) {
        if (_send(buffer[i])) n++;
    }
    return n;
}

/**
 * @brief               Listens to incomming data from the module until timeout or END_OF_RESPONSE.
 * @return              Number of bytes received and stored in the internal _buffer.
 * @protected
 */
int8_t AllWize::_receive() {
    return _readBytesUntil(END_OF_RESPONSE, (char*) _buffer, RX_BUFFER_SIZE);
}

/**
 * @brief               Sends a binary buffer and waits for response. Returns the number of bytes received and stored in the internal _buffer.
 * @param buffer        Binary data to send
 * @param len           Length of the binary data
 * @return              Number of bytes received, -1 if timed out or error sending
 * @protected
 */
int8_t AllWize::_sendAndReceive(uint8_t * buffer, uint8_t len) {
    if (_send(buffer, len) != len) return -1;
    return _receive();
}

/**
 * @brief               Sends a byte and waits for response. Returns the number of bytes received and stored in the internal _buffer.
 * @param ch            Byte to send (-1 if timed out)
 * @return              Number of bytes received, -1 if timed out or error sending
 * @protected
 */
int8_t AllWize::_sendAndReceive(uint8_t ch) {
    if (_send(ch) != 1) return -1;
    return _receive();
}

// -----------------------------------------------------------------------------
// Utils
// -----------------------------------------------------------------------------

/**
 * @brief               Reads a byte from the stream with a timeout
 * @return              Read char or -1 if timed out
 * @protected
 */
int AllWize::_timedRead() {
    uint32_t _start = millis();
    while (millis() - _start < _timeout) {
        int ch = _stream->read();
        if (ch >= 0) return ch;
        #if defined(ARDUINO_ARCH_ESP8266)
            yield();
        #endif
    };
    return -1;
}

/**
 * @brief               Reads the stream buffer up to a number of bytes
 * @param data          Buffer to store the values to
 * @param len           Max number of bytes to read
 * @return              Number of bytes read or -1 if timed out
 * @protected
 */
int AllWize::_readBytes(char * data, uint16_t len) {
    if (len < 1) return 0;
    uint16_t index = 0;
    while (index < len) {
        int ch = _timedRead();
        if (ch < 0) break;

        #if defined(ALLWIZE_DEBUG_PORT)
        {
            char buffer[10];
            snprintf(buffer, sizeof(buffer), "r %02X '%c'", ch, (32 <= ch && ch <= 126) ? ch : 32);
            ALLWIZE_DEBUG_PRINTLN(buffer);
        }
        #endif

        *data++ = (char) ch;
        index++;
    }
    return index;
}

/**
 * @brief               Reads the stream buffer up to a certain char or times out
 * @param terminator    Terminating char
 * @param data          Buffer to store the values to
 * @param len           Max number of bytes to read
 * @return              Number of bytes read or -1 if timed out
 * @protected
 */
int AllWize::_readBytesUntil(char terminator, char * data, uint16_t len) {
    if (len < 1) return 0;
    uint16_t index = 0;
    while (index < len) {

        int ch = _timedRead();
        if (ch < 0) break;

        #if defined(ALLWIZE_DEBUG_PORT)
        {
            char buffer[10];
            snprintf(buffer, sizeof(buffer), "r %02X '%c'", ch, (32 <= ch && ch <= 126) ? ch : 32);
            ALLWIZE_DEBUG_PRINTLN(buffer);
        }
        #endif

        if (ch == terminator) break;
        *data++ = (char) ch;
        index++;
    }
    return index;
}

/**
 * @brief               Converts a hex c-string to a binary buffer.
 * @param hex           C-string with the hex values
 * @param bin           Buffer to store the converted values in
 * @param len           Length of the hex c-string
 * @protected
 */
void AllWize::_hex2bin(char * hex, uint8_t * bin, uint8_t len) {
    for (uint8_t i=0; i<len; i+=2) {
        bin[i/2] = ((hex[i] - '0') * 16 + (hex[i+1] - '0')) & 0xFF;
    }
}

/**
 * @brief               Converts a binary buffer to an hex c-string.
 * @param bin           Buffer to read the values from
 * @param hex           C-string to store the hex values
 * @param len           Length of the input buffer
 * @protected
 */
void AllWize::_bin2hex(uint8_t * bin, char * hex, uint8_t len) {
    for (uint8_t i=0; i<len; i++) {
        sprintf(&hex[i*2], "%02X", bin[i]);
    }
}