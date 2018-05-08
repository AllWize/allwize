/*

RC1701XX module mockup

Copyright (C) 2016-2018 by Xose Pérez <xose dot perez at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#include <stdlib.h>
#include <Stream.h>

#define MOCKUP_RESPONSE_BYTE        0xA0
#define MOCKUP_DEBUG                0
#define MOCKUP_BUFFER_SIZE          128

class CircularBuffer {

    public:

        CircularBuffer(uint8_t size = 128) : _size(size) {
            _buffer = new char[size];
        }

        virtual ~CircularBuffer() {
            delete[] _buffer;
        }

        virtual void flush() {
            _read = _write = 0;
        }

        virtual uint8_t available() {
            if (_write >= _read) return _write - _read;
            return _size - _read + _write;
        }

        virtual int write(char ch) {
            _buffer[_write] = ch;
            _write = (_write + 1) % _size;
            return 1;
        }

        virtual int read() {
            if (_write == _read) return -1;
            uint8_t ch = _buffer[_read];
            _read = (_read + 1) % _size;
            return ch;
        }

        virtual int peek() {
            if (_write == _read) return -1;
            return (uint8_t) _buffer[_read];
        }

    private:
        char * _buffer;
        uint8_t _size;
        uint8_t _read;
        uint8_t _write;

};

class RC1701XX_Mockup : public Stream {

    public:

        // ---------------------------------------------------------------------
        // Constructor & destructor
        // ---------------------------------------------------------------------

        RC1701XX_Mockup() {
            _rx = new CircularBuffer(MOCKUP_BUFFER_SIZE);
            _tx = new CircularBuffer(MOCKUP_BUFFER_SIZE);
        }

        virtual ~RC1701XX_Mockup() {
            delete _rx;
            delete _tx;
        }

        virtual void reset() {
            _pending_payload = 0;
            _pending_response = 0;
            _config_mode = false;
            _memory_mode = false;
            _command_mode = false;
            _rx->flush();
            _tx->flush();
        }

        // ---------------------------------------------------------------------
        // Stream interface
        // ---------------------------------------------------------------------

        virtual size_t write(uint8_t ch) {

            #if MOCKUP_DEBUG
                Serial.print("Received: 0x");
                Serial.println((uint8_t) ch, HEX);
            #endif

            _process(ch);
            return _rx->write(ch);

        }

        virtual int read() {
            return _tx->read();
        }

        virtual int available() {
            return _tx->available();
        }

        virtual int peek() {
            return _tx->peek();
        }

        virtual void flush() {
            _tx->flush();
        }

        // ---------------------------------------------------------------------
        // Inverted stream
        // ---------------------------------------------------------------------

        virtual size_t rx_write(uint8_t ch) {

            #if MOCKUP_DEBUG
                Serial.print("Sending : 0x");
                Serial.println((uint8_t) ch, HEX);
            #endif

            return _tx->write(ch);

        }

        virtual int rx_read() {
            return _rx->read();
        }

        virtual int rx_available() {
            return _rx->available();
        }

        virtual void rx_flush() {
            _rx->flush();
        }

    private:

        // ---------------------------------------------------------------------
        // Data processing
        // ---------------------------------------------------------------------

        void _process(uint8_t ch) {

            // Expected payload sizes (defaults to 1 byte)
            if (0 == _pending_payload) {

                // Check config mode
                if (!_config_mode & (0x00 == ch)) _config_mode = true;
                if (_config_mode & (0x58 == ch)) _config_mode = false;
                if (!_config_mode) return;
                if (_command_mode) return;

                // Memory mode
                if (_memory_mode) {
                    if (0xFF == ch) {
                        _memory_mode = false;
                        rx_write('>');
                    } else {
                        _pending_payload = 1;
                    }
                    return;
                }

                // Handle cases
                switch (ch) {

                    case 0x00:
                        _pending_payload = 0;
                        break;

                    case '@':
                        _command_mode = true;
                        return;

                    case 'A':
                        _pending_payload = 2;
                        break;

                    case 'B':
                        _pending_payload = 8;
                        break;

                    case 'K':
                        _pending_payload = 17;
                        break;

                    case 'L':
                        _pending_payload = 1;
                        _pending_response = 8;
                        break;

                    case 'M':
                        _memory_mode = true;
                        _pending_payload = 2;
                        break;

                    case 'O':
                        _pending_payload = 1;
                        _pending_response = 2;
                        break;

                    case 'Q':
                    case 'S':
                    case 'U':
                    case 'V':
                        _pending_payload = 0;
                        rx_write(MOCKUP_RESPONSE_BYTE);
                        break;

                    case 'T':
                        _pending_payload = 8;
                        break;

                    case 'W':
                        // TODO
                        break;

                    case 'Y':
                        _pending_payload = 1;
                        _pending_response = 1;
                        break;

                    default:
                        _pending_payload = 1;
                        break;

                }

                // Show prompt
                rx_write('>');

            } else {

                // Update pending payload
                --_pending_payload;

                // Memory mode
                if (_memory_mode || _command_mode) return;

                // If no more payload
                if (0 == _pending_payload) {

                    // Inject response
                    for (uint8_t i=0; i<_pending_response; i++) {
                        rx_write(MOCKUP_RESPONSE_BYTE);
                    }

                    // Reset response size
                    _pending_response = 0;

                    // Show prompt
                    rx_write('>');

                }

            }
        }

        CircularBuffer * _rx;  // Chars sent to the module (using write)
        CircularBuffer * _tx;  // Chars sent by the module (read-able)

        uint8_t _pending_payload = 0;
        uint8_t _pending_response = 0;
        bool _config_mode = false;
        bool _memory_mode = false;
        bool _command_mode = false;

};
