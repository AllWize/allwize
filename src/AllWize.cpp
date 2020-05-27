/*

AllWize Library

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

/**
 * @file AllWize.cpp
 * AllWize library code file
 */

#include "AllWize.h"
#include <assert.h>

// -----------------------------------------------------------------------------
// Init
// -----------------------------------------------------------------------------

/**
 * @brief               AllWize object constructor
 * @param serial        HardwareSerial object to communicate with the module
 * @param reset_gpio    GPIO connected to the module RESET pin
 * @param config_gpio   GPIO connected to the module CONFIG pin
 */
AllWize::AllWize(HardwareSerial *serial, uint8_t reset_gpio, uint8_t config_gpio) : _stream(serial), _hw_serial(serial), _reset_gpio(reset_gpio), _config_gpio(config_gpio) {
    _init();
}

#if not defined(ARDUINO_ARCH_SAMD) && not defined(ARDUINO_ARCH_ESP32)
/**
 * @brief               AllWize object constructor
 * @param serial        SoftwareSerial object to communicate with the module
 * @param reset_gpio    GPIO connected to the module RESET pin
 * @param config_gpio   GPIO connected to the module CONFIG pin
 */
AllWize::AllWize(SoftwareSerial *serial, uint8_t reset_gpio, uint8_t config_gpio) : _stream(serial), _sw_serial(serial), _reset_gpio(reset_gpio), _config_gpio(config_gpio) {
    _init();
}
#endif

/**
 * @brief               AllWize object constructor
 * @param rx            GPIO for RX
 * @param tx            GPIO for TX
 * @param reset_gpio    GPIO connected to the module RESET pin
 * @param config_gpio   GPIO connected to the module CONFIG pin
 */
AllWize::AllWize(uint8_t rx, uint8_t tx, uint8_t reset_gpio, uint8_t config_gpio) : _rx(rx), _tx(tx), _reset_gpio(reset_gpio), _config_gpio(config_gpio) {
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
    if (GPIO_NONE != _config_gpio) {
        pinMode(_config_gpio, OUTPUT);
        digitalWrite(_config_gpio, LOW);
    }
    randomSeed(analogRead(0));
    _access_number = random(0, 256);
}

/**
 * @brief               Inits the module communications
 */
void AllWize::begin(uint8_t baudrate) {
    
    _baudrate = BAUDRATES[baudrate-1];
    reset();
    _niceDelay(200);
    
    // Figure out module type
    _readModel();
    String part_number = getPartNumber();
    if (part_number.equals("RC1701HP-MBUS4")) {
        _module = MODULE_MBUS4;
    } else if (part_number.equals("RC1701HP-OSP")) {
        _module = MODULE_OSP;
    } else if (part_number.equals("RC1701HP-WIZE")) {
        _module = MODULE_WIZE;
        _ci = CI_WIZE;
    } else {
        _module = MODULE_UNKNOWN;
    }

    _append_rssi = _getSlot(MEM_RSSI_MODE) == 0x01;
    _mbus_mode = _getSlot(MEM_MBUS_MODE);
    _data_interface = _getSlot(MEM_DATA_INTERFACE);
    
}

/**
 * @brief               Resets the serial object
 */
void AllWize::_resetSerial() {

    if (_hw_serial) {

        _hw_serial->end();
#if defined(ARDUINO_ARCH_ESP32)
        if ((_rx != -1) && (_tx != -1)) {
            pinMode(_rx, FUNCTION_4);
            pinMode(_tx, FUNCTION_4);
            _hw_serial->begin(_baudrate, SERIAL_8N1, _rx, _tx);
        } else {
            _hw_serial->begin(_baudrate);
        }
#else
        _hw_serial->begin(_baudrate);
#endif

    } else {

#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_SAMD)
        // It should never hit this block
        assert(false);
#else
        _sw_serial->end();
        _sw_serial->begin(_baudrate);
#endif

    }

    _flush();

    // Cache memory
    #if USE_MEMORY_CACHE
        _ready = _cacheMemory(_memory);
    #endif

}

/**
 * @brief               Resets the radio module.
 * @return              Reset successfully issued
 */
bool AllWize::reset() {
    if (GPIO_NONE == _reset_gpio) {
        _resetSerial();
        _niceDelay(100);
        _setSlot(MEM_CONFIG_INTERFACE, 1);
        if (_setConfig(true)) {
            _send('@');
            _send('R');
            _send('R');
            _niceDelay(100);
            if (GPIO_NONE != _config_gpio) {
                digitalWrite(_config_gpio, LOW);
            }
            _config = false;
            _resetSerial();
            return true;
        }
    } else {
        digitalWrite(_reset_gpio, LOW);
        _niceDelay(1);
        digitalWrite(_reset_gpio, HIGH);
        _niceDelay(100);
        if (GPIO_NONE != _config_gpio) {
            digitalWrite(_config_gpio, LOW);
        }
        _config = false;
        _resetSerial();
        return true;
    }
    return false;
}

/**
 * @brief               Cleans the RX/TX line
 */
void AllWize::softReset() {
    if (_setConfig(true)) _setConfig(false);
    /*
    if (_send(CMD_ENTER_CONFIG) == 1) {
        _flush();
        _send(CMD_EXIT_CONFIG);
    }
    _niceDelay(10);
    */
}

/**
 * @brief               Resets the module to factory settings
 * @return              Factory reset successfully issued
 */
bool AllWize::factoryReset() {
    _resetSerial();
    _niceDelay(100);
    _setSlot(MEM_CONFIG_INTERFACE, 1);
    if (_setConfig(true)) {
        _send('@');
        _send('R');
        _send('C');
        _niceDelay(100);
        if (GPIO_NONE != _config_gpio) {
            digitalWrite(_config_gpio, LOW);
        }
        _config = false;
        _resetSerial();
        return true;
    }
    return false;
}

/**
 * @brief               Sets the module in master mode
 */
void AllWize::master() {
    setMode(DEFAULT_MBUS_MODE, true);
    setNetworkRole(NETWORK_ROLE_MASTER);
    setInstallMode(INSTALL_MODE_HOST);
    setSleepMode(SLEEP_MODE_DISABLE);
    setPower(POWER_20dBm);
    setDataRate(DATARATE_2400bps);
    setDataInterface(DATA_INTERFACE_START_STOP);
    setAppendRSSI(true);
    setControlField(C_ACK);
}

/**
 * @brief               Sets the module in slave mode
 */
void AllWize::slave() {
    setMode(DEFAULT_MBUS_MODE, true);
    setNetworkRole(NETWORK_ROLE_SLAVE);
    setPower(POWER_20dBm);
    setDataRate(DATARATE_2400bps);
    setControlField(C_SND_NR);
}

/**
 * @brief               Sets the module in repeater mode
 */
void AllWize::repeater() {
    setMode(DEFAULT_MBUS_MODE, true);
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
    _niceDelay(5);
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
        _niceDelay(100);
    }
    return false;
}

/**
 * @brief               Dumps the current memory configuration to the given stream
 * @param debug         Data stream to dump the data to
 */
void AllWize::dump(Stream &debug) {

    #if not USE_MEMORY_CACHE
        uint8_t _memory[0x100] = {0xFF};
        bool _ready = _cacheMemory(_memory);
    #endif

    if (!_ready) {
        debug.println("Error doing memory dump...");
        return;
    }

    char ch[10];
    char ascii[17] = {0};
    uint8_t address = 0;
    ascii[16] = 0;

    debug.println();
    debug.print("       ");
    for (address = 0; address <= 0x0F; address++) {
        snprintf(ch, sizeof(ch), "%02X ", address);
        debug.print(ch);
    }
    debug.println();
    debug.print("------------------------------------------------------");

    address = 0;
    while (true) {
        
        if ((address % 16) == 0) {
            if (address > 0)
                debug.print(ascii);
            snprintf(ch, sizeof(ch), "\n0x%02X:  ", address);
            debug.print(ch);
        }
        if ((31 < _memory[address]) && (_memory[address] < 127)) {
            ascii[address % 16] = (char)_memory[address];
        } else {
            ascii[address % 16] = ' ';
        }
        snprintf(ch, sizeof(ch), "%02X ", (uint8_t)_memory[address]);
        debug.print(ch);
        
        if (0xFF == address) break;
        address++;

    }

    debug.println();
    debug.println();

}

/**
 * @brief               Sends a byte array
 * @param buffer        Byte array with the application payload
 * @param len           Length of the payload
 * @return              Returns true if message has been correctly sent
 */
bool AllWize::send(uint8_t *buffer, uint8_t len) {
 
    // Check we are in IDLE mode
    if (_config) return false;

    // Clean line
    softReset();

    // Send no response message if len is 0
    if (0 == len) return (1 == _send(0xFE));

    // Wize transport layer
    bool send_wize_transport_layer = (MODULE_WIZE == _module) && (CI_WIZE == _ci);

    // message length is payload length + 1 (CI) + 2 (for timestamp if wize) + 5 (wize transport layer if wize)
    uint8_t message_len = len + 1;
    if (MODULE_WIZE == _module) message_len += 2;
    if (send_wize_transport_layer) message_len += 5;

    // max payload size is 0xF6 bytes
    if (message_len > 0xF6) return false;

    // length
    if (1 != _send(message_len)) return false;

    // control information field
    if (1 != _send(_ci)) return false;

    // transport layer
    if (send_wize_transport_layer) {
        _send(_wize_control & 0xFF);        // Wize Control
        _send(_wize_network_id & 0xFF);     // Network ID HIGH
        _send((_counter >> 0) & 0xFF);      // Frame counter LOW
        _send((_counter >> 8) & 0xFF);      // Frame counter HIGH
        _send(_wize_application);           // Wize app indicator
    }

    // application payload
    if (len != _send(buffer, len)) return false;

    // timestamp, TODO: add option to provide a timestamp
    if (MODULE_WIZE == _module) {
        _send(0);
        _send(0);
    }

    _access_number++;
    _counter++;
    return true;

}

/**
 * @brief               Sends c-string
 * @param buffer        C-string with the application payload
 * @return              Returns true if message has been correctly sent
 */
bool AllWize::send(const char *buffer) {
    return send((uint8_t *)buffer, strlen(buffer));
}

/**
 * @brief               Sends an ACK
 * @return              Returns true if message has been correctly sent
 */
bool AllWize::ack() {
    if (_config) return false;
    setControlField(C_ACK);
    return send("ACK");
}

/**
 * @brief               Enables or disables RF recever
 * @param enable        True to enable, false to disable
 * @return              Returns true if successfully set
 */
bool AllWize::enableRX(bool enable) {
    if (_config) return false;
    if (enable) {
        _send(CMD_IDLE_ENABLE_RF);
    } else {
        _send(CMD_IDLE_DISABLE_RF);
    }
    return true;
}

/**
 * @brief               Returns true if a new message has been received and decoded
 *                      This method has to be called in the main loop to monitor for incoming messages
 * @return              Whether a new message is available
 */
bool AllWize::available() {

    bool response = false;

    static uint32_t when = millis();

    while (_stream->available() && _pointer < RX_BUFFER_SIZE) {

        uint8_t ch = _stream->read();

        #if defined(ALLWIZE_DEBUG_PORT)
        {
            char buffer[10];
            snprintf(buffer, sizeof(buffer), "r %02X '%c'", ch, (32 <= ch && ch <= 126) ? ch : 32);
            ALLWIZE_DEBUG_PRINTLN(buffer);
        }
        #endif

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
        
        // If we don't soft-reset the line the RX channel gets stalled
        softReset();

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

/**
 * @brief               Sets the wize control field in the transpoprt layer
 * @param wize_control  Wize Control (defined the key to be used)
 * @return              True is correctly set
 */
bool AllWize::setWizeControl(uint8_t wize_control) {
    if (wize_control > 14) return false;
    _wize_control = wize_control;
    return true;
}

/**
 * @brief                   Use AllWize::setWizeNetworkId instead
 * @deprecated
 */
void AllWize::setWizeOperatorId(uint8_t wize_network_id) { 
    setWizeNetworkId(wize_network_id);
}

/**
 * @brief                   Sets the wize network ID field in the transpoprt layer
 * @param wize_network_id   Wize Network ID
 */
void AllWize::setWizeNetworkId(uint8_t wize_network_id) {
    _wize_network_id = wize_network_id;
}

/**
 * @brief                   Sets the wize applicaton field in the transpoprt layer
 * @param wize_application  Wize Application
 */
void AllWize::setWizeApplication(uint8_t wize_application) {
    _wize_application = wize_application;
}

/**
 * @brief               Sets the wize couonter field in the transpoprt layer
 * @param counter       Wize counter
 */
void AllWize::setCounter(uint16_t counter) { 
    _counter = counter; 
}

/**
 * @brief               Gets the current wize counter
 * @return              Counter
 */
uint16_t AllWize::getCounter() { 
    return _counter; 
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
    if (channel > 41) return;
    if (persist) {
        _setSlot(MEM_CHANNEL, channel);
        if (MODULE_WIZE == _module) {
            _setSlot(MEM_CHANNEL_RX, channel);
        }
    }
    _sendCommand(CMD_CHANNEL, channel);
}

/**
 * @brief               Gets the channel stored in non-volatile memory
 * @return              Channel (1 byte)
 */
uint8_t AllWize::getChannel() {
    return _getSlot(MEM_CHANNEL);
}

/**
 * @brief               Sets the RF power
 * @param power         Value from 1 to 5
 * @param persist       Persist the changes in non-volatile memory (defaults to False)
 */
void AllWize::setPower(uint8_t power, bool persist) {
    if (0 < power && power < 6) {
        if (persist) {
            _setSlot(MEM_RF_POWER, power);
        }
        _sendCommand(CMD_RF_POWER, power);
    }
}

/**
 * @brief               Gets the RF power stored in non-volatile memory
 * @return              RF power (1 byte)
 */
uint8_t AllWize::getPower() {
    return _getSlot(MEM_RF_POWER);
}

/**
 * @brief               Sets the data rate
 * @param dr            Value in [1, 2, 3, 4, 5]
 */
void AllWize::setDataRate(uint8_t dr) {

    if (_module == MODULE_MBUS4) return;
    if (dr < 1) return;
    if (_module == MODULE_OSP) {
        if (DATARATE_6400bps == dr) {
            dr = DATARATE_6400bps_OSP;
        }
        if (dr > 5) return;
    }
    if (_module == MODULE_WIZE) {
        if (dr > 3) return;
    }

    _setSlot(MEM_DATA_RATE, dr);
    if (MODULE_WIZE == _module) {
        _setSlot(MEM_DATA_RATE_RX, dr);
    }

}

/**
 * @brief               Gets the data rate stored in non-volatile memory
 * @return              Current data rate (1 byte)
 */
uint8_t AllWize::getDataRate() {
    return _getSlot(MEM_DATA_RATE);
}

/**
 * @brief               Sets the module in one of the available MBus modes
 * @param mode          MBus mode (MBUS_MODE_*)
 * @param persist       Persist the changes in non-volatile memory (defaults to False)
 */
void AllWize::setMode(uint8_t mode, bool persist) {

    // Wize FW accepts only modes 0x10 and 0x11
    if (MODULE_WIZE == _module) {
        if ((MBUS_MODE_N1 != mode) && (MBUS_MODE_N2 != mode)) return;
    }
    
    // Only OSP FW accepts mode 0x12
    if ((MBUS_MODE_OSP == mode) && (MODULE_OSP != _module)) return;

    if (persist) {
        _setSlot(MEM_MBUS_MODE, mode);
    }
    _sendCommand(CMD_MBUS_MODE, mode);
    _mbus_mode = mode;

}

/**
 * @brief               Gets the MBus mode stored in non-volatile memory
 * @return              MBus mode (1 byte)
 */
uint8_t AllWize::getMode() {
    return _mbus_mode;
}

/**
 * @brief               Sets the sleep mode
 * @param mode          One of SLEEP_MODE_*
 */
void AllWize::setSleepMode(uint8_t mode) {
    _setSlot(MEM_SLEEP_MODE, mode);
}

/**
 * @brief               Gets the sleep mode stored in non-volatile memory
 * @return              Sleep mode (1 byte)
 */
uint8_t AllWize::getSleepMode() {
    return _getSlot(MEM_SLEEP_MODE);
}

/**
 * @brief               Sets the RSSI mode value
 * @param value         Set to true to append RSSI value to received data
 */
void AllWize::setAppendRSSI(bool value) {
    if (value == 1) {
        _setSlot(MEM_RSSI_MODE, 1);
    } else {
        _setSlot(MEM_RSSI_MODE, 0);
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
        _setSlot(MEM_PREAMBLE_LENGTH, preamble);
    }
}

/**
 * @brief               Gets the preamble length frame format
 * @return              Preamble length format (1 byte)
 */
uint8_t AllWize::getPreamble() {
    return _getSlot(MEM_PREAMBLE_LENGTH);
}

/**
 * @brief               Sets the buffer timeout (also used for auto sleep modes)
 * @param ms            Timeout value in milliseconds
 */
void AllWize::setTimeout(uint16_t ms) {
    if (ms > 4080) return;
    uint8_t timeout = (ms / 16) - 1;
    _setSlot(MEM_TIMEOUT, timeout);
}

/**
 * @brief               Gets the current buffer timeout (also used for auto sleep modes)
 * @return              Timeout setting in millis
 */
uint16_t AllWize::getTimeout() {
    uint8_t timeout = _getSlot(MEM_TIMEOUT);
    return 16 * (uint16_t) (timeout + 1);
}

/**
 * @brief               Sets the network role
 * @param role          Network role (NETWORK_ROLE_*)
 */
void AllWize::setNetworkRole(uint8_t role) {
    _setSlot(MEM_NETWORK_ROLE, role);
}

/**
 * @brief               Gets the current network role
 * @return              Network role
 */
uint8_t AllWize::getNetworkRole() {
    return _getSlot(MEM_NETWORK_ROLE);
}

/**
 * @brief               Sets the LED control
 * @param value         LED control value
 */
void AllWize::setLEDControl(uint8_t value) {
    if (value > 3) return;
    _setSlot(MEM_LED_CONTROL, value);
}

/**
 * @brief               Gets the current LED control
 * @return              LED control value
 */
uint8_t AllWize::getLEDControl() {
    return _getSlot(MEM_LED_CONTROL);
}

/**
 * @brief               Sets the data interface for receiving packets
 * @param value         Value from 0x00 to 0x0C
 */
void AllWize::setDataInterface(uint8_t value) {
    if (value <= 0x0C) {
        _setSlot(MEM_DATA_INTERFACE, value);
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
        _setSlot(MEM_CONTROL_FIELD, value);
    }
    _sendCommand(CMD_CONTROL_FIELD, value);
}

/**
 * @brief               Gets the control field value stored in non-volatile memory
 * @return              Control field value (1 byte)
 */
uint8_t AllWize::getControlField() {
    return _getSlot(MEM_CONTROL_FIELD);
}

/**
 * @brief               Sets the module in one of the available operations modes
 * @param mode          Operation mode
 * @param persist       Persist the changes in non-volatile memory (defaults to False)
 */
void AllWize::setInstallMode(uint8_t mode, bool persist) {
    if (mode <= 2) {
        if (persist) {
            _setSlot(MEM_INSTALL_MODE, mode);
        }
        _sendCommand(CMD_INSTALL_MODE, mode);
    }
}

/**
 * @brief               Gets the install modevalue stored in non-volatile memory
 * @return              Install mode value (1 byte)
 */
uint8_t AllWize::getInstallMode() {
    return _getSlot(MEM_INSTALL_MODE);
}

/**
 * @brief               Sets the encrypt flag setting
 * @param flag          Encrypt flag
 */
void AllWize::setEncryptFlag(uint8_t flag) {
    if (0 == flag || 1 == flag || 3 == flag) {
        _setSlot(MEM_ENCRYPT_FLAG, flag);
    }
}

/**
 * @brief               Gets the encrypt flag setting
 * @return              Encrypt flag
 */
uint8_t AllWize::getEncryptFlag() {
    return _getSlot(MEM_ENCRYPT_FLAG);
}

/**
 * @brief               Sets the decrypt flag setting
 * @param flag          Decrypt flag
 */
void AllWize::setDecryptFlag(uint8_t flag) {
    _setSlot(MEM_DECRYPT_FLAG, flag);
}

/**
 * @brief               Gets the decrypt flag setting
 * @return              Decrypt flag
 */
uint8_t AllWize::getDecryptFlag() {
    return _getSlot(MEM_DECRYPT_FLAG);
}

/**
 * @brief               Sets the default encryption key
 * @param reg           Register number (1-64)
 * @param key           A 16-byte encryption key as binary array
 */
void AllWize::setKey(uint8_t reg, const uint8_t *key) {
    if (reg > 128) return;
    uint8_t data[17];
    data[0] = reg;
    memcpy(&data[1], key, 16);
    _sendCommand(CMD_KEY_REGISTER, data, 17);
}

/**
 * @brief               Sets the default encryption key
 * @param key           A 16-byte encryption key as binary array
 */
void AllWize::setDefaultKey(const uint8_t *key) {
    _setSlot(MEM_DEFAULT_KEY, (uint8_t *)key, 16);
}

/**
 * @brief               Gets the default encryption key
 * @param key           A binary buffer to store the key (16 bytes)
 */
void AllWize::getDefaultKey(uint8_t *key) {
    _getSlot(MEM_DEFAULT_KEY, key, 16);
}

/**
 * @brief               Sets new/specific access number.
 * @param value         New access number
 */
void AllWize::setAccessNumber(uint8_t value) {
    _sendCommand(CMD_ACCESS_NUMBER, value);
}

/**
 * @brief               Sets the UART baud rate, requires reset to take effect
 * @param value         Value from 1 to 11
 */
void AllWize::setBaudRate(uint8_t value) {
    if ((0 < value) & (value < 12)) {
        if (ready()) _setSlot(MEM_UART_BAUD_RATE, value);
        _baudrate = BAUDRATES[value-1];
    }
}

/**
 * @brief               Gets the UART baud rate
 * @return              Value (1 byte)
 */
uint8_t AllWize::getBaudRate() {
    return _getSlot(MEM_UART_BAUD_RATE);
}

/**
 * @brief               Gets the UART baud rate speed in bps
 * @param value         Baudrate code
 * @return              UART speed
 */
uint32_t AllWize::getBaudRateSpeed(uint8_t value) {
    if ((0 < value) & (value < 12)) {
        return BAUDRATES[value-1];
    }
    return 0;
}

// -----------------------------------------------------------------------------

/**
 * @brief               Returns the RSSI of the last valid packet received
 *                      TODO: values do not seem right and are not the same as in the packet
 * @return              RSSI in dBm
 */
float AllWize::getRSSI() {
    uint8_t response = _sendCommand(CMD_RSSI);
    if (response > 0) return -0.5 * (float) _buffer[0];
    return 0;
}

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
 * @brief               Returns the Manufacturer ID
 * @return              Manufacturer ID
 */
String AllWize::getMID() {
    return _getSlotAsHexString(MEM_MANUFACTURER_ID, 2);
}

/**
 * @brief               Sets the Manufacturer ID
 * @param mid           MID to save
 */
bool AllWize::setMID(uint16_t mid) {
    uint8_t buffer[2];
    buffer[0] = (mid >> 8) & 0xFF;
    buffer[1] = (mid >> 0) & 0xFF;
    return _setSlot(MEM_MANUFACTURER_ID, buffer, 2);
}

/**
 * @brief               Returns the Unique ID string
 * @return              4-byte hex string with the unique ID
 */
String AllWize::getUID() {
    return _getSlotAsHexString(MEM_UNIQUE_ID, 4);
}

/**
 * @brief               Saved the UID into the module memory
 * @param uid           UID to save
 */
bool AllWize::setUID(uint32_t uid) {
    uint8_t buffer[4];
    buffer[0] = (uid >> 24) & 0xFF;
    buffer[1] = (uid >> 16) & 0xFF;
    buffer[2] = (uid >>  8) & 0xFF;
    buffer[3] = (uid >>  0) & 0xFF;
    return _setSlot(MEM_UNIQUE_ID, buffer, 4);
}

/**
 * @brief               Returns the device version from non-volatile memory
 * @return              Version
 */
uint8_t AllWize::getVersion() {
    return _getSlot(MEM_VERSION);
}

/**
 * @brief               Sets the device version
 * @param version       Device version
 */
void AllWize::setVersion(uint8_t version) {
    _setSlot(MEM_VERSION, version);
}

/**
 * @brief               Returns the device type from non-volatile memory
 * @return              Device
 */
uint8_t AllWize::getDevice() {
    return _getSlot(MEM_DEVICE);
}

/**
 * @brief               Sets the device type
 * @param type          Device type
 */
void AllWize::setDevice(uint8_t type) {
    _setSlot(MEM_DEVICE, type);
}

/**
 * @brief               Returns the module part number
 * @return              12-byte hex string with the part number
 */
String AllWize::getPartNumber() {
    return _model;
}

/**
 * @brief               Returns the minimum required hardware version to run the current firmware
 * @return              4-byte hex string with the HW version
 */
String AllWize::getRequiredHardwareVersion() {
    return _hw;
}

/**
 * @brief               Returns the module firmware revision
 * @return              4-byte hex string with the FW version
 */
String AllWize::getFirmwareVersion() {
    return _fw;
}

/**
 * @brief               Returns the module serial number
 * @return              8-byte hex string with the serial number
 */
String AllWize::getSerialNumber() {
    return _getSlotAsHexString(MEM_SERIAL_NUMBER, 8);
}

/**
 * @brief               Returns the module type
 * @return              One of MODULE_UNKNOWN, MODULE_MBUS4, MODULE_OSP and MODULE_WIZE
 */
uint8_t AllWize::getModuleType() {
    return _module;
}

/**
 * @brief               Returns the module type
 * @return              One of MODULE_UNKNOWN, MODULE_MBUS4, MODULE_OSP and MODULE_WIZE
 */
String AllWize::getModuleTypeName() {
    switch (_module) {
        case MODULE_MBUS4: return String("MBUS4");
        case MODULE_OSP: return String("OSP");
        case MODULE_WIZE: return String("WIZE");
    }
    return String("Unknown");
}


/**
 * @brief               Returns the frequency for the given channel
 * @param channel       Channel
 * @return              Frequency (float) in MHz for the given channel
 */
double AllWize::getFrequency(uint8_t channel) {
    if (channel <   7) {
        return 169.40625 + 0.0125 * (channel - 1);
    } else if (channel ==  7) {
        return 169.41250;
    } else if (channel ==  8) {
        return 169.43750;
    } else if (channel ==  9) {
        return 169.46250;
    } else if (channel == 10) {
        return 169.43750;
    } else if (channel <  38) {
        return 169.48125 + 0.0125 * (channel - 11);
    } else if (channel <  42) {
        return 169.62500 + 0.0500 * (channel - 38);
    } else {
        return 0;
    }
}

/**
 * @brief               Returns the speed for te given datarate
 * @param dr            Datarate
 * @return              Speed in bps
 */
uint16_t AllWize::getDataRateSpeed(uint8_t dr) {
    if (dr == DATARATE_6400bps_OSP) dr = DATARATE_6400bps;
    if ((0 < dr) && (dr < 5)) {
        return DATARATES[dr-1];
    }
    return 0;
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
            if (GPIO_NONE != _config_gpio) {
                digitalWrite(_config_gpio, HIGH);
                _config = true;
            } else {
                _config = (_sendAndReceive(CMD_ENTER_CONFIG) == 0);
            }
        } else {
            if (GPIO_NONE != _config_gpio) {
                digitalWrite(_config_gpio, LOW);
            }
            _send(CMD_EXIT_CONFIG);
            _niceDelay(5);
            _config = false;
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
int8_t AllWize::_sendCommand(uint8_t command, uint8_t *data, uint8_t len) {
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

// ------------------------------------------------------------------------------------------------

/**
 * @brief               Reads and caches the module memory
 * @param buffer        Buffer with at least 256 positions to hold memory
 * @return              True if successfully read
 * @protected
 */
bool AllWize::_cacheMemory(uint8_t * buffer) {

    // Read memory
    _setConfig(true);
    _send(CMD_TEST_MODE_0);
    bool ret = (256 == _readBytes((char *) buffer, 256));
    _setConfig(false);
    return ret;

}

/**
 * @brief               Searches for the module model
 * @protected
 */
void AllWize::_readModel() {

    #if not USE_MEMORY_CACHE
        uint8_t _memory[0x100] = {0xFF};
        bool _ready = _cacheMemory(_memory);
        if (!_ready) return;
    #endif

    // Look for the part_number
    bool found = false;
    uint8_t index = 0;
    uint8_t len = strlen(MODULE_SIGNATURE);
    for (index=0; index<0xFF-32; index++) {
        if (memcmp(&_memory[index], (uint8_t *) MODULE_SIGNATURE, len) == 0) {
            found = true;
            break;
        }
    }

    // Parse signature
    if (found) {

        String part_number = String((char *) &_memory[index]);
        part_number.substring(0, 31);
        part_number.trim();

        uint8_t end = part_number.indexOf(",");
        _model = part_number.substring(0, end);
        uint8_t start = end + 1;
        end = part_number.indexOf(",", start);
        _hw = part_number.substring(start, end);
        _fw = part_number.substring(end + 1, 32);
        _fw.trim();

    }

}

/**
 * @brief               Returns the contents of consecutive memory addresses
 * @param address       Starting memory address
 * @param buffer        Buffer with at least 'len' position to store data to
 * @param len           Number of positions to read
 * @return              Number of positions actually read
 * @protected
 */
uint8_t AllWize::_getMemory(uint8_t address, uint8_t *buffer, uint8_t len) {
    #if USE_MEMORY_CACHE
        if (!_ready) return 0;
        memcpy(buffer, &_memory[address], len);
        return len;
    #else
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
    #endif
}

/**
 * @brief               Returns the contents of memory address
 * @param address       Memory address
 * @return              Contents
 * @protected
 */
uint8_t AllWize::_getMemory(uint8_t address) {
    #if USE_MEMORY_CACHE
        if (!_ready) return 0;
        return _memory[address];
    #else
        uint8_t response = _sendCommand(CMD_READ_MEMORY, address);
        if (response > 0) return _buffer[0];
        return 0;
    #endif
}

/**
 * @brief               Sets non-volatile memory contents starting from given address
 * @param address       Memory address
 * @param data          Single byte to store at given address
 * @return              True if the data was successfully saved
 * @protected
 */
bool AllWize::_setMemory(uint8_t address, uint8_t data) {

    // Check cached data
    #if USE_MEMORY_CACHE
        if (_memory[address] == data) return true;
    #endif

    // Build query buffer
    uint8_t buffer[3] = {address, data, (uint8_t)CMD_EXIT_MEMORY};
    
    // Execute command
    bool ret = (_sendCommand(CMD_WRITE_MEMORY, buffer, 3) != -1);

    // Update cached memory
    #if USE_MEMORY_CACHE
        if (ret) _memory[address] = data;
    #endif

    return ret;

}

/**
 * @brief               Sets non-volatile memory contents starting from given address
 * @param address       Memory address
 * @param data          Binary data to store
 * @param len           Length of the binary data
 * @return              True if the data was successfully saved
 * @protected
 */
bool AllWize::_setMemory(uint8_t address, uint8_t *data, uint8_t len) {
    
    // Check cached data
    #if USE_MEMORY_CACHE
        if (memcmp(&_memory[address], data, len) == 0) return true;
    #endif

    // Build query buffer
    uint8_t buffer[len * 2 + 1];
    for (uint8_t i = 0; i < len; i++) { 
        buffer[i * 2] = address + i;
        buffer[i * 2 + 1] = data[i];
    }
    buffer[len * 2] = CMD_EXIT_MEMORY;
    
    // Execute command
    bool ret = (_sendCommand(CMD_WRITE_MEMORY, buffer, len * 2 + 1) != -1);
    
    // Update cached memory
    #if USE_MEMORY_CACHE
        if (ret) memcpy(&_memory[address], data, len);
    #endif

    return ret;

}

// ------------------------------------------------------------------------------------------------

/**
 * @brief               Return the physical memory address for the given slot
 * @param  slot         Memory slot
 * @return              An address, 0xFF if not found or non existant
 * @protected
 */
uint8_t AllWize::_getAddress(uint8_t slot) {
    if ((slot >= MEM_MAX_SLOTS) || (MODULE_UNKNOWN == _module)) {
        return 0xFF;
    }
    return MEM_ADDRESS[_module-1][slot];
}

/**
 * @brief               Sets non-volatile memory contents starting from given address
 * @param slot          Memory slot
 * @param data          Binary data to store
 * @param len           Length of the binary data
 * @return              True if the data was successfully saved
 * @protected
 */
bool AllWize::_setSlot(uint8_t slot, uint8_t *data, uint8_t len) {
    uint8_t address = _getAddress(slot);
    if (0xFF == address) return false;
    return _setMemory(address, data, len);
}

/**
 * @brief               Sets non-volatile memory contents starting from given address
 * @param slot          Memory slot
 * @param data          Single byte to store at given address
 * @return              True if the data was successfully saved
 * @protected
 */
bool AllWize::_setSlot(uint8_t slot, uint8_t data) {
    uint8_t address = _getAddress(slot);
    if (0xFF == address) return false;
    return _setMemory(address, data);
}

/**
 * @brief               Returns the contents of consecutive memory addresses
 * @param slot          Memory slot
 * @param buffer        Buffer with at least 'len' position to store data to
 * @param len           Number of positions to read
 * @return              Number of positions actually read
 * @protected
 */
uint8_t AllWize::_getSlot(uint8_t slot, uint8_t *buffer, uint8_t len) {
    uint8_t address = _getAddress(slot);
    if (0xFF == address) return 0;
    return _getMemory(address, buffer, len);
}

/**
 * @brief               Returns the contents of single-byte memory slot
 * @param slot          Memory slot
 * @return              Contents of the slot, 0 if error
 * @protected
 */
uint8_t AllWize::_getSlot(uint8_t slot) {
    uint8_t address = _getAddress(slot);
    if (0xFF == address) return 0;
    return _getMemory(address);
}

/**
 * @brief               Returns the contents of the memory from a certain address as an HEX String
 * @param slot          Memory slot
 * @param len           Number of bytes to read
 * @return              Result (empty string if error)
 * @protected
 */
String AllWize::_getSlotAsHexString(uint8_t slot, uint8_t len) {
    uint8_t bin[len];
    char hex[2 * len + 1];
    hex[0] = 0;
    if (len == _getSlot(slot, bin, len)) {
        _bin2hex(bin, hex, len);
    }
    return String(hex);
}

/**
 * @brief               Returns the contents of the memory from a certain address as a String object
 * @param slot          Memory slot
 * @param len           Number of bytes to read
 * @return              Result (empty string if error)
 * @protected
 */
String AllWize::_getSlotAsString(uint8_t slot, uint8_t len) {
    uint8_t bin[len];
    char hex[len + 1];
    hex[0] = 0;
    if (len == _getSlot(slot, bin, len)) {
        memcpy(hex, bin, len);
        hex[len - 1] = 0;
    }
    return String(hex);
}

// ------------------------------------------------------------------------------------------------

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

    // Get current values
    uint8_t mbus_mode = getMode();
    uint8_t data_interface = getDataInterface();
    bool has_start = (data_interface & 0x04) == 0x04;
    bool has_header = (mbus_mode != MBUS_MODE_OSP) & ((data_interface & 0x01) == 0x00);
    bool has_rssi = getAppendRSSI();
    bool has_crc = (data_interface & 0x08) == 0x08;
    uint8_t bytes_not_in_len = has_start ? 3 : 1;
    uint8_t bytes_not_in_app = (has_header ? 9 : 0) + 1 + (has_rssi ? 1 : 0) + (has_crc ? 2 : 0);
    uint8_t bytes_not_in_msg = 0;

    // This variable will contain the pointer to the current reading position
    uint8_t in = 0;

    // Start byte
    if (has_start) {
        if (START_BYTE != _buffer[in++]) return false;
    };

    // Get and check buffer length
    uint8_t len = _buffer[in++];
    if (_pointer != len + bytes_not_in_len) return false;

    if (has_header) {

        // C-field
        _message.c = _buffer[in++];

        // Manufacturer
        uint16_t man = (_buffer[in + 1] << 8) + _buffer[in];
        _message.man[0] = ((man >> 10) & 0x001F) + 64;
        _message.man[1] = ((man >> 5) & 0x001F) + 64;
        _message.man[2] = ((man >> 0) & 0x001F) + 64;
        _message.man[3] = 0;
        in += 2;

        // Address
        _message.address[0] = _buffer[in + 3];
        _message.address[1] = _buffer[in + 2];
        _message.address[2] = _buffer[in + 1];
        _message.address[3] = _buffer[in + 0];
        in += 4;

        // Version
        _message.version = _buffer[in++];

        // Type
        _message.type = _buffer[in++];

    } else {
        _message.c = 0xFF;
        _message.type = 0;
        _message.version = 0;
        _message.man[0] = 0;
        memset(_message.address, 0, 6);
    }

    // Control information
    _message.ci = _buffer[in++];

    // Wize transport layer
    if (MODULE_WIZE == _module) {
        
        if (CI_WIZE == _message.ci) {
        
            bytes_not_in_app += 5;
            
            // Wize control
            _message.wize_control = _buffer[in++];

            // Wize operator ID
            _message.wize_network_id = _buffer[in++];

            // Wize counter
            _message.wize_counter = (_buffer[in + 1] << 8) + _buffer[in];
            in += 2;

            // Wize application
            _message.wize_application = _buffer[in++];

        } else {
            
            // Undocumented hack 
            bytes_not_in_msg = 8;
        
        }
    
    }

    // Application data
    _message.len = len - bytes_not_in_app - bytes_not_in_msg;
    memcpy(_message.data, &_buffer[in], _message.len);
    _message.data[_message.len] = 0;
    in += (_message.len + bytes_not_in_msg);

    // RSSI
    if (has_rssi) {
        _message.rssi = _buffer[in++];
    } else {
        _message.rssi = 0xFF;
    }

    // CRC
    if (has_crc) {
        in += 2;
    }

    // Stop byte
    if (has_start) {
        if (STOP_BYTE != _buffer[in]) return false;
    }

    return true;

}

// -----------------------------------------------------------------------------

/**
 * @brief               Flushes the serial line to the module
 * @protected
 */
void AllWize::_flush() {

    // Flush TX line
    _stream->flush();

    // Flush RX line
    while (_stream->available()) _stream->read();

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
uint8_t AllWize::_send(uint8_t *buffer, uint8_t len) {
    uint8_t n = 0;
    for (uint8_t i = 0; i < len; i++) {
        if (_send(buffer[i])) n++;
    }
    return n;
}

/**
 * @brief               Listens to incoming data from the module until timeout or END_OF_RESPONSE.
 * @return              Number of bytes received and stored in the internal _buffer.
 * @protected
 */
int8_t AllWize::_receive() {
    return _readBytesUntil(END_OF_RESPONSE, (char *) _buffer, RX_BUFFER_SIZE);
}

/**
 * @brief               Sends a binary buffer and waits for response. Returns the number of bytes received and stored in the internal _buffer.
 * @param buffer        Binary data to send
 * @param len           Length of the binary data
 * @return              Number of bytes received, -1 if timed out or error sending
 * @protected
 */
int8_t AllWize::_sendAndReceive(uint8_t *buffer, uint8_t len) {
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

/**
 * @brief               Reads a byte from the stream with a timeout
 * @return              Read char or -1 if timed out
 * @protected
 */
int AllWize::_timedRead() {

    uint32_t _start = millis();
    int ch = -1;
    while (millis() - _start < _timeout) {
        #if defined(ARDUINO_ARCH_ESP8266)
            yield();
        #endif
        ch = _stream->read();
        if (ch >= 0) break;
    };

    #if defined(ALLWIZE_DEBUG_PORT)
    /*
    {
        if (ch < 0) {
            ALLWIZE_DEBUG_PRINTLN("r TIMEOUT");
        } else {
            char buffer[10];
            snprintf(buffer, sizeof(buffer), "r %02X '%c'", ch, (32 <= ch && ch <= 126) ? ch : 32);
            ALLWIZE_DEBUG_PRINTLN(buffer);
        }
    }
    */
    #endif

    return ch;
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
        if (ch < 0) return -1;
        *data++ = (char)ch;
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
int AllWize::_readBytesUntil(char terminator, char *data, uint16_t len) {
    
    if (len < 1) return 0;
    
    uint16_t index = 0;
    while (index < len) {
        int ch = _timedRead();
        if (ch < 0) return -1;
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
void AllWize::_hex2bin(char *hex, uint8_t *bin, uint8_t len) {
    for (uint8_t i = 0; i < len; i += 2) {
        bin[i / 2] = ((hex[i] - '0') * 16 + (hex[i + 1] - '0')) & 0xFF;
    }
}

/**
 * @brief               Converts a binary buffer to an hex c-string.
 * @param bin           Buffer to read the values from
 * @param hex           C-string to store the hex values
 * @param len           Length of the input buffer
 * @protected
 */
void AllWize::_bin2hex(uint8_t *bin, char *hex, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        sprintf(&hex[i * 2], "%02X", bin[i]);
    }
}

/**
 * @brief               Does a non-blocking delay
 * @param ms            milliseconds to delay
 * @protected
 */
void AllWize::_niceDelay(uint32_t ms) {
    uint32_t start = millis();
    while (millis() - start < ms) delay(1);
}