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
// Public
// -----------------------------------------------------------------------------

/**
 * Allwize object constructor
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
 * Inits the module communications
 */
void Allwize::begin() {

    //  Start serial interface.
    //_stream.begin(MODEM_BAUDRATE);
    delay(200);
    _stream.flush();

}

/**
 * Timeout in ms for module readings
 * @param {uint32_t} timeout    Timeout in milliseconds
 */
//void Allwize::setTimeout(uint32_t timeout) {
    //_stream.timeout(timeout);
//}

/**
 * Resets the radio module
 * Datasheet says "Minimum 250 ns pulse width"
 */
void Allwize::reset() {
    if (0xFF != _reset_gpio) {
        digitalWrite(_reset_gpio, LOW);
        delay(10);
        digitalWrite(_reset_gpio, HIGH);
    }
}

/**
 * Sets the communications channel
 * @param {uint8_t} channel     Channel number
 */
void Allwize::setChannel(uint8_t channel) {
    _sendCommand(CMD_CHANNEL, channel);
}

// -----------------------------------------------------------------------------
// Protected
// -----------------------------------------------------------------------------

/**
 * Sets or unsets config mode
 * @param {bool} value      True to enter config mode
 * @protected
*/
bool Allwize::_setConfig(bool value) {
    if (value != _config) {
        if (value) {
            _sendWait(CMD_ENTER_CONFIG);
        } else {
            _send(CMD_EXIT_CONFIG);
        }
        _config = value;
    }
    return _config;
}

/**
 * Sends a command with the given data
 * @param {uint8_t} command     Command key
 * @param {uint8_t *} data      Binary data to send
 * @param {size_t} len          Length of the binary data
 * @protected
 */
void Allwize::_sendCommand(uint8_t command, uint8_t * data, size_t len) {
    _setConfig(true);
    _sendWait(command);
    _sendWait(data, len);
    _setConfig(false);
}

/**
 * Sends a command with the given data
 * @param {uint8_t} command     Command key
 * @param {uint8_t} data        Single byte
 * @protected
 */
void Allwize::_sendCommand(uint8_t command, uint8_t data) {
    _setConfig(true);
    _sendWait(command);
    _sendWait(data);
    _setConfig(false);
}

/**
 * Sets non-volatile memory contents starting from given address
 * @param {uint8_t} address     Command key
 * @param {uint8_t *} data      Binary data to store
 * @param {size_t} len          Length of the binary data
 * @protected
 */
void Allwize::_setMemory(uint8_t address, uint8_t * data, size_t len) {
    uint8_t buffer[len*2+1];
    for (uint8_t i=0; i<len; i++) {
        buffer[i*2]   = address + i;
        buffer[i*2+1] = data[i];
    }
    buffer[len*2] = 0xFF;
    _sendCommand(CMD_MEMORY, buffer, len*2+1);
}

/**
 * Sets non-volatile memory contents starting from given address
 * @param {uint8_t} address     Command key
 * @param {uint8_t} data        Single byte to store at given address
 * @protected
 */
void Allwize::_setMemory(uint8_t address, uint8_t value) {
    uint8_t buffer[3] = {address, value, 0xFF};
    _sendCommand(CMD_MEMORY, buffer, 3);
}

// -----------------------------------------------------------------------------

/**
 * Flushes the serial line to the module
 */
void Allwize::_flush() {
    _stream.flush();
}

/**
 * Sends a single byte to the module UART.
 * Returns the number of bytes actually sent.
 * @param {uint8_t} ch          Byte to send
 * @return size_t
 * @protected
 */
size_t Allwize::_send(uint8_t ch) {
    return _stream.write(ch);
}

/**
 * Sends a binary buffer to the module UART.
 * Returns the number of bytes actually sent.
 * @param {uint8_t *} buffer    Binary data to send
 * @param {size_t} len          Length of the binary data
 * @return size_t
 * @protected
 */
size_t Allwize::_send(uint8_t * buffer, size_t len) {
    size_t n = 0;
    for (uint8_t i=0; i<len; i++) {
        if (_send(buffer[i])) n++;
    }
    return n;
}

/**
 * Listens to incomming data from the module until timeout or END_OF_RESPONSE.
 * Returns the number of bytes received and stored in the internal _buffer.
 * @return size_t
 * @protected
 */
size_t Allwize::_receive() {
    return _stream.readBytesUntil(END_OF_RESPONSE, (char*) _buffer, RX_BUFFER_SIZE);
}

/**
 * Sends a binary buffer and waits for response.
 * Returns the number of bytes received and stored in the internal _buffer.
 * @param {uint8_t *} buffer    Binary data to send
 * @param {size_t} len          Length of the binary data
 * @protected
 */
size_t Allwize::_sendWait(uint8_t * buffer, size_t len) {
    _send(buffer, len);
    return _receive();
}

/**
 * Sends a byte and waits for response.
 * Returns the number of bytes received and stored in the internal _buffer.
 * @param {uint8_t} ch          Byte to send
 * @protected
 */
size_t Allwize::_sendWait(uint8_t ch) {
    _send(ch);
    return _receive();
}

// -----------------------------------------------------------------------------
// Utils
// -----------------------------------------------------------------------------

/**
 * Converts a hex c-string to a binary buffer.
 * @param {char *} hex          C-string with the hex values
 * @param {uint8_t *} bin       Buffer to store the converted values in
 * @param {size_t} len          Length of the hex c-string
 * @protected
 */
void Allwize::_hex2bin(char * hex, uint8_t * bin, size_t len) {
    for (uint8_t i=0; i<len; i+=2) {
        bin[i/2] = ((hex[i] - '0') * 16 + (hex[i+1] - '0')) & 0xFF;
    }
}

/**
 * Converts a binary buffer to an hex c-string.
 * @param {uint8_t *} bin       Buffer to read the values from
 * @param {char *} hex          C-string to store the hex values
 * @param {size_t} len          Length of the input buffer
 * @protected
 */
void Allwize::_bin2hex(uint8_t * bin, char * hex, size_t len) {
    for (uint8_t i=0; i<len; i++) {
        hex[i*2]   = ((bin[i] >> 4) & 0x0F) + '0';
        hex[i*2+1] = ((bin[i]     ) & 0x0F) + '0';
    }
}
