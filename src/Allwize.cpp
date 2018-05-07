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

#include "Allwize.h"

// -----------------------------------------------------------------------------
// Init
// -----------------------------------------------------------------------------

/**
 * @brief Allwize object constructor
 * @param {Stream&} stream      Serial stream to communicate with the module
 * @param {uint8_t} _reset_gpio GPIO connected to the module RESET pin
 */
Allwize::Allwize(Stream& stream, uint8_t reset_gpio) : _stream(stream), _reset_gpio(reset_gpio) {
    if (0xFF != _reset_gpio) {
        pinMode(_reset_gpio, OUTPUT);
        digitalWrite(_reset_gpio, HIGH);
    }
}

/**
 * @brief Inits the module communications
 */
void Allwize::begin() {
    delay(200);
    _stream.flush();
}

/**
 * @brief Resets the radio module.
 */
void Allwize::reset() {
    if (0xFF != _reset_gpio) {
        digitalWrite(_reset_gpio, LOW);
        delay(1);
        digitalWrite(_reset_gpio, HIGH);
    } else {
        _setMemory(MEM_CONFIG_INTERFACE, 1);
        if (_setConfig(true)) {
            _send('@');
            _send('R');
            _send('R');
        }
    }
}

/**
 * @brief Resets the module to factory settings
 */
void Allwize::factoryReset() {
    _setMemory(MEM_CONFIG_INTERFACE, 1);
    if (_setConfig(true)) {
        _send('@');
        _send('R');
        _send('C');
    }
}

/**
 * @brief Sets the module in master mode
 */
void Allwize::master() {
    setNetworkRole(NETWORK_ROLE_MASTER);
    setInstallMode(INSTALL_MODE_HOST);
}

/**
 * @brief Sets the module in slave mode
 */
void Allwize::slave() {
    setNetworkRole(NETWORK_ROLE_SLAVE);
}

/**
 * @brief Sets the module in repeater mode
 */
void Allwize::repeater() {
    setNetworkRole(NETWORK_ROLE_REPEATER);
}

/**
 * @brief Sets the radio module in sleep mode
 */
void Allwize::sleep() {
    _sendCommand(CMD_SLEEP);
}

/**
 * @brief Wakes up the radio from sleep mode
 */
void Allwize::wakeup() {
    _sendAndReceive(CMD_EXIT_CONFIG);
}

/**
 * @brief Test whether the radio module is ready or not
 */
bool Allwize::ready() {
    bool response = _setConfig(true);
    if (response) _setConfig(false);
    return response;
}

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

/**
 * @brief Sets the communications channel (for MBUS_MODE_R2 only)
 * @param {uint8_t} channel     Channel number
 * @param {bool} persist        Persist the changes in non-volatile memory
 */
void Allwize::setChannel(uint8_t channel, bool persist) {
    if (persist) {
        _setMemory(MEM_CHANNEL, channel);
    } else {
        _sendCommand(CMD_CHANNEL, channel);
    }
}

/**
 * @brief Gets the channel stored in non-volatile memory
 * @return {uint8_t} Channel (1 byte)
 */
uint8_t Allwize::getChannel() {
    return _getMemory(MEM_CHANNEL);
}

/**
 * @brief Sets the RF power
 * @param {uint8_t} power       Value from 1 to 5
 * @param {bool} persist        Persist the changes in non-volatile memory
 */
void Allwize::setPower(uint8_t power, bool persist) {
    if (0 < power && power < 6) {
        if (persist) {
            _setMemory(MEM_RF_POWER, power);
        } else {
            _sendCommand(CMD_RF_POWER, power);
        }
    }
}

/**
 * @brief Gets the RF power stored in non-volatile memory
 * @return {uint8_t} RF power (1 byte)
 */
uint8_t Allwize::getPower() {
    return _getMemory(MEM_RF_POWER);
}

/**
 * @brief Sets the module in one of the available MBus modes
 * @param {uint8_t} mode        MBus mode
 * @param {bool} persist        Persist the changes in non-volatile memory
 */
void Allwize::setMBusMode(uint8_t mode, bool persist) {
    if (persist) {
        _setMemory(MEM_MBUS_MODE, mode);
    } else {
        _sendCommand(CMD_MBUS_MODE, mode);
    }
}

/**
 * @brief Gets the MBus mode stored in non-volatile memory
 * @return {uint8_t} MBus mode (1 byte)
 */
uint8_t Allwize::getMBusMode() {
    return _getMemory(MEM_MBUS_MODE);
}

/**
 * @brief Sets the sleep mode
 * @param {uint8_t} mode        One of SLEEP_MODE_*
 */
void Allwize::setSleepMode(uint8_t mode) {
    _setMemory(MEM_SLEEP_MODE, mode);
}

/**
 * @brief Gets the sleep mode stored in non-volatile memory
 * @return {uint8_t} Sleep mode (1 byte)
 */
uint8_t Allwize::getSleepMode() {
    return _getMemory(MEM_SLEEP_MODE);
}

/**
 * @brief Sets the RSSI mode value
 * @param {bool} value          Set to true to append RSSI value to received data
 */
void Allwize::setAppendRSSI(bool value) {
    _setMemory(MEM_RSSI_MODE, value ? 1 : 0);
}

/**
 * @brief Gets the current RSSI mode value
 * @return {bool} True if RSSI value will be appended to received data
 */
bool Allwize::getAppendRSSI() {
    return (_getMemory(MEM_RSSI_MODE) == 0x01);
}

/**
 * @brief Sets the timeout for auto sleep modes
 * @param {uint8_t} timeout     Timeout value
 */
void Allwize::setTimeout(uint8_t timeout) {
    _setMemory(MEM_TIMEOUT, timeout);
}

/**
 * @brief Gets the current timeout for auto sleep modes
 * @return {uint8_t}            Timeout setting
 */
uint8_t Allwize::getTimeout() {
    return _getMemory(MEM_TIMEOUT);
}

/**
 * @brief Sets the network role
 * @param {uint8_t} role        Network role
 */
void Allwize::setNetworkRole(uint8_t role) {
    _setMemory(MEM_NETWORK_ROLE, role);
}

/**
 * @brief Gets the current network role
 * @return {uint8_t}            Network role
 */
uint8_t Allwize::getNetworkRole() {
    return _getMemory(MEM_NETWORK_ROLE);
}

/**
 * @brief Sets the LED control
 * @param {uint8_t} value       LED control value
 */
void Allwize::setLEDControl(uint8_t value) {
    _setMemory(MEM_LED_CONTROL, value);
}

/**
 * @brief Gets the current LED control
 * @return {uint8_t}            LED control value
 */
uint8_t Allwize::getLEDControl() {
    return _getMemory(MEM_LED_CONTROL);
}

/**
 * @brief Sets the control field value
 * @param {uint8_t} value       Control field
 * @param {bool} persist        Persist the changes in non-volatile memory
 */
void Allwize::setControlField(uint8_t value, bool persist) {
    if (persist) {
        _setMemory(MEM_CONTROL_FIELD, value);
    } else {
        _sendCommand(CMD_CONTROL_FIELD, value);
    }
}

/**
 * @brief Gets the control field value stored in non-volatile memory
 * @return {uint8_t} Control field value (1 byte)
 */
uint8_t Allwize::getControlField() {
    return _getMemory(MEM_CONTROL_FIELD);
}

/**
 * @brief Sets the module in one of the available operations modes
 * @param {allwize_install_mode_t} mode Operation mode
 */
void Allwize::setInstallMode(uint8_t mode, bool persist) {
    if (persist) {
        _setMemory(MEM_INSTALL_MODE, mode);
    } else {
        _sendCommand(CMD_INSTALL_MODE, mode);
    }
}

/**
 * @brief Gets the install modevalue stored in non-volatile memory
 * @return {uint8_t} Install mode value (1 byte)
 */
uint8_t Allwize::getInstallMode() {
    return _getMemory(MEM_INSTALL_MODE);
}

/**
 * @brief Sets the encrypt flag setting
 * @param {uint8_t} flag        Encrypt flag
 */
void Allwize::setEncryptFlag(uint8_t flag) {
    _setMemory(MEM_ENCRYPT_FLAG, flag);
}

/**
 * @brief Gets the encrypt flag setting
 * @return {uint8_t}            Encrypt flag
 */
uint8_t Allwize::getEncryptFlag() {
    return _getMemory(MEM_ENCRYPT_FLAG);
}

/**
 * @brief Sets the decrypt flag setting
 * @param {uint8_t} flag        Decrypt flag
 */
void Allwize::setDecryptFlag(uint8_t flag) {
    _setMemory(MEM_DECRYPT_FLAG, flag);
}

/**
 * @brief Gets the decrypt flag setting
 * @return {uint8_t}            Decrypt flag
 */
uint8_t Allwize::getDecryptFlag() {
    return _getMemory(MEM_DECRYPT_FLAG);
}

/**
 * @brief Sets the default encryption key
 * @param {uint8_t *} key      A 16-byte encryption key as binary array
 */
void Allwize::setDefaultKey(uint8_t * key) {
    _setMemory(MEM_DEFAULT_KEY, key, 16);
}

/**
 * @brief Gets the default encryption key
 * @param {uint8_t *} key      A binary buffer to store the key (16 bytes)
 */
void Allwize::getDefaultKey(uint8_t * key) {
    _getMemory(MEM_DEFAULT_KEY, key, 16);
}

// -----------------------------------------------------------------------------

/**
 * @brief Returns the RSSI of the last valid packet received
 * @return {float} RSSI in dBm
 */
float Allwize::getRSSI() {
    uint8_t response = _sendCommand(CMD_RSSI);
    if (response > 0) return -0.5 * _buffer[0];
    return 0;
}

/**
 * @brief Returns the internal temperature of the module
 * @return {uint8_t} Temperature in Celsius
 */
uint8_t Allwize::getTemperature() {
    uint8_t response = _sendCommand(CMD_TEMPERATURE);
    if (response > 0) return (_buffer[0] - 128);
    return 0;
}

/**
 * @brief Returns the internal voltage of the module
 * @return {uint16_t} Voltage in mV
 */
uint16_t Allwize::getVoltage() {
    uint8_t response = _sendCommand(CMD_VOLTAGE);
    if (response > 0) return 30 * _buffer[0];
    return 0;
}

/**
 * @brief Returns the Manufacturer ID string
 * @return {String} 2-byte hex string with the manufacturer ID
 */
String Allwize::getMID() {
    return _getMemoryAsHexString(MEM_MANUFACTURER_ID, 2);
}

/**
 * @brief Returns the Unique ID string
 * @return {String} 4-byte hex string with the unique ID
 */
String Allwize::getUID() {
    return _getMemoryAsHexString(MEM_UNIQUE_ID, 4);
}

/**
 * @brief Returns the module version from non-volatile memory
 * @return {uint8_t} Version
 */
uint8_t Allwize::getVersion() {
    return _getMemory(MEM_VERSION);
}

/**
 * @brief Returns the device version from non-volatile memory
 * @return {uint8_t} Version
 */
uint8_t Allwize::getDevice() {
    return _getMemory(MEM_DEVICE);
}

/**
 * @brief Returns the module part number
 * @return {String} 12-byte hex string with the part number
 */
String Allwize::getPartNumber() {
    return _getMemoryAsHexString(MEM_PART_NUMBER, 12);
}

/**
 * @brief Returns the module hardware revision
 * @return {String} 4-byte hex string with the HW version
 */
String Allwize::getHardwareVersion() {
    return _getMemoryAsHexString(MEM_HW_REV_NUMBER, 4);
}

/**
 * @brief Returns the module firmware revision
 * @return {String} 4-byte hex string with the FW version
 */
String Allwize::getFirmwareVersion() {
    return _getMemoryAsHexString(MEM_FW_REV_NUMBER, 4);
}

/**
 * @brief Returns the module serial number
 * @return {String} 8-byte hex string with the serial number
 */
String Allwize::getSerialNumber() {
    return _getMemoryAsHexString(MEM_SERIAL_NUMBER, 8);
}

// -----------------------------------------------------------------------------
// Protected
// -----------------------------------------------------------------------------

/**
 * @brief Sets or unsets config mode
 * @param {bool} value      True to enter config mode
 * @return {bool}           True if in config mode
 * @protected
*/
bool Allwize::_setConfig(bool value) {
    if (value != _config) {
        if (value) {
            if (_sendAndReceive(CMD_ENTER_CONFIG) == 0) {
                _config = true;
            }
        } else {
            _send(CMD_EXIT_CONFIG);
            _config = false;
        }
    }
    return _config;
}

/**
 * @brief Sends a command with the given data
 * @param {uint8_t} command     Command key
 * @param {uint8_t *} data      Binary data to send
 * @param {uint8_t} len         Length of the binary data
 * @protected
 */
int8_t Allwize::_sendCommand(uint8_t command, uint8_t * data, uint8_t len) {
    int8_t response = -1;
    if (!_setConfig(true)) return response;
    if (_sendAndReceive(command) != -1) {
        response = _sendAndReceive(data, len);
    }
    _setConfig(false);
    return response;
}

/**
 * @brief Sends a command with the given data
 * @param {uint8_t} command     Command key
 * @param {uint8_t} data        Single byte
 * @return {int8_t}             Number of bytes received, -1 if timed out or error sending
 * @protected
 */
int8_t Allwize::_sendCommand(uint8_t command, uint8_t data) {
    int8_t response = -1;
    if (!_setConfig(true)) return response;
    if (_sendAndReceive(command) != -1) {
        response = _sendAndReceive(data);
    }
    _setConfig(false);
    return response;
}

/**
 * @brief Sends a command with no data
 * @param {uint8_t} command     Command key
 * @return {int8_t}             Number of bytes received, -1 if timed out or error sending
 * @protected
 */
int8_t Allwize::_sendCommand(uint8_t command) {
    int8_t response = -1;
    if (!_setConfig(true)) return response;
    response = _sendAndReceive(command);
    _setConfig(false);
    return response;
}

/**
 * @brief Sets non-volatile memory contents starting from given address
 * @param {uint8_t} address     Command key
 * @param {uint8_t *} data      Binary data to store
 * @param {uint8_t} len         Length of the binary data
 * @return {bool}               True if the data was successfully saved
 * @protected
 */
bool Allwize::_setMemory(uint8_t address, uint8_t * data, uint8_t len) {
    uint8_t buffer[len*2+1];
    for (uint8_t i=0; i<len; i++) {
        buffer[i*2]   = address + i;
        buffer[i*2+1] = data[i];
    }
    buffer[len*2] = CMD_EXIT_MEMORY;
    return (_sendCommand(CMD_WRITE_MEMORY, buffer, len*2+1) != -1);
}

/**
 * @brief Sets non-volatile memory contents starting from given address
 * @param {uint8_t} address     Command key
 * @param {uint8_t} data        Single byte to store at given address
 * @return {bool}               True if the data was successfully saved
 * @protected
 */
bool Allwize::_setMemory(uint8_t address, uint8_t value) {
    uint8_t buffer[3] = {address, value, CMD_EXIT_MEMORY};
    return (_sendCommand(CMD_WRITE_MEMORY, buffer, 3) != -1);
}

/**
 * @brief Returns the contents of consecutive memory addresses
 * @param {uint8_t} address     Address to start from
 * @param {uint8_t *} buffer    Buffer with at least 'len' position to store data to
 * @param {uint8_t} len         Number of positions to read
 * @return {uint8_t}            Number of positions actually read
 * @protected
 */
uint8_t Allwize::_getMemory(uint8_t address, uint8_t * buffer, uint8_t len) {
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
 * @brief Returns the contents of single memory addresses
 * @param {uint8_t} address     Address to start from
 * @return {uint8_t}            Contents of the address, 0 if error
 * @protected
 */
uint8_t Allwize::_getMemory(uint8_t address) {
    uint8_t response = _sendCommand(CMD_READ_MEMORY, address);
    if (response > 0) return _buffer[0];
    return 0;
}

/**
 * @brief Returns the contents of the memory from a certain address as an HEX String
 * @param {uint8_t} address     Address to start from
 * @param {uint8_t} len         Number of bytes to read
 * @return {String}             Result
 * @protected
 */
String Allwize::_getMemoryAsHexString(uint8_t address, uint8_t len) {
    uint8_t bin[len] = {0};
    char hex[2*len+1] = {0};
    _getMemory(address, bin, len);
    _bin2hex(bin, hex, len);
    return String(hex);
}

// -----------------------------------------------------------------------------

/**
 * @brief Flushes the serial line to the module
 * @protected
 */
void Allwize::_flush() {
    _stream.flush();
}

/**
 * @brief Sends a single byte to the module UART. Returns the number of bytes actually sent.
 * @param {uint8_t} ch          Byte to send
 * @return {uint8_t}            Number of bytes actually sent
 * @protected
 */
uint8_t Allwize::_send(uint8_t ch) {
    return _stream.write(ch);
}

/**
 * @brief Sends a binary buffer to the module UART. Returns the number of bytes actually sent.
 * @param {uint8_t *} buffer    Binary data to send
 * @param {uint8_t} len         Length of the binary data
 * @return {uint8_t}            Number of bytes actually sent
 * @protected
 */
uint8_t Allwize::_send(uint8_t * buffer, uint8_t len) {
    uint8_t n = 0;
    for (uint8_t i=0; i<len; i++) {
        if (_send(buffer[i])) n++;
    }
    return n;
}

/**
 * @brief Listens to incomming data from the module until timeout or END_OF_RESPONSE. Returns the number of bytes received and stored in the internal _buffer.
 * @return {int8_t}             Number of bytes received, -1 if timed out or error sending
 * @protected
 */
int8_t Allwize::_receive() {
    return _readBytesUntil(END_OF_RESPONSE, (char*) _buffer, RX_BUFFER_SIZE);
}

/**
 * @brief Sends a binary buffer and waits for response. Returns the number of bytes received and stored in the internal _buffer.
 * @param {uint8_t *} buffer    Binary data to send
 * @param {uint8_t} len         Length of the binary data
 * @return {int8_t}             Number of bytes received, -1 if timed out or error sending
 * @protected
 */
int8_t Allwize::_sendAndReceive(uint8_t * buffer, uint8_t len) {
    if (_send(buffer, len) != len) return -1;
    return _receive();
}

/**
 * @brief Sends a byte and waits for response. Returns the number of bytes received and stored in the internal _buffer.
 * @param {uint8_t} ch          Byte to send (-1 if timed out)
 * @return {int8_t}             Number of bytes received, -1 if timed out or error sending
 * @protected
 */
int8_t Allwize::_sendAndReceive(uint8_t ch) {
    if (_send(ch) != 1) return -1;
    return _receive();
}

// -----------------------------------------------------------------------------
// Utils
// -----------------------------------------------------------------------------

/**
 * @brief Reads a byte from the stream with a timeout
 * @returns {int}               Read char or -1 if timed out
 * @protected
 */
int Allwize::_timedRead() {
    uint32_t _start = millis();
    while (millis() - _start < _timeout) {
        int ch = _stream.read();
        if (ch >= 0) return ch;
    };
    return -1;
}

/**
 * @brief Reads the stream buffer up to a certain char or times out
 * @param {char} terminator     Terminating char
 * @param {char *} buffer       Buffer to store the values to
 * @param {uint8_t} len         Max number of bytes to read
 * @returns {int}               Number of bytes read or -1 if timed out
 * @protected
 */
int Allwize::_readBytesUntil(char terminator, char * buffer, uint8_t len) {
    if (len < 1) return 0;
    size_t index = 0;
    while (index < len) {
        int ch = _timedRead();
        if (ch < 0) return ch;
        if (ch == terminator) break;
        *buffer++ = (char) ch;
        index++;
    }
    return index;
}

/**
 * @brief Converts a hex c-string to a binary buffer.
 * @param {char *} hex          C-string with the hex values
 * @param {uint8_t *} bin       Buffer to store the converted values in
 * @param {uint8_t} len         Length of the hex c-string
 * @protected
 */
void Allwize::_hex2bin(char * hex, uint8_t * bin, uint8_t len) {
    for (uint8_t i=0; i<len; i+=2) {
        bin[i/2] = ((hex[i] - '0') * 16 + (hex[i+1] - '0')) & 0xFF;
    }
}

/**
 * @brief Converts a binary buffer to an hex c-string.
 * @param {uint8_t *} bin       Buffer to read the values from
 * @param {char *} hex          C-string to store the hex values
 * @param {uint8_t} len         Length of the input buffer
 * @protected
 */
void Allwize::_bin2hex(uint8_t * bin, char * hex, uint8_t len) {
    for (uint8_t i=0; i<len; i++) {
        sprintf(&hex[i*2], "%02X", bin[i]);
    }
}
