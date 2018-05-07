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

class CircularBuffer {

    public:

        CircularBuffer(uint8_t size = 128) : _size(size) {
            _buffer = new char[size];
        }

        ~CircularBuffer() {
            delete[] _buffer;
        }

        void flush() {
            _read = _write = 0;
        }

        uint8_t available() {
            if (_write >= _read) return _write - _read;
            return _size - _read + _write;
        }

        int write(char ch) {
            _buffer[_write] = ch;
            _write = (_write + 1) % _size;
            return 1;
        }

        int read() {
            if (_write == _read) return -1;
            uint8_t ch = _buffer[_read];
            _read = (_read + 1) % _size;
            return ch;
        }

        int peek() {
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
            _rx = new CircularBuffer(128);
            _tx = new CircularBuffer(128);
        }

        ~RC1701XX_Mockup() {
            delete _rx;
            delete _rx;
        }

        // ---------------------------------------------------------------------
        // Stream interface
        // ---------------------------------------------------------------------

        virtual size_t write(uint8_t ch) {
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

        void _process(char ch) {

            static uint8_t pending_payload = 0;
            static uint8_t response_size = 0;
            static bool config_mode = false;

            // Expected payload sizes (defaults to 1 byte)
            if (0 == pending_payload) {

                // Check config mode
                if (!config_mode & (0x00 == ch)) config_mode = true;
                if (config_mode & (0x58 == ch)) config_mode = false;
                if (!config_mode) return;

                // Handle cases
                switch (ch) {

                    case 0x00:
                        pending_payload = 0;
                        break;

                    case 'A':
                        pending_payload = 2;
                        break;

                    case 'B':
                        pending_payload = 8;
                        break;

                    case 'K':
                        pending_payload = 17;
                        break;

                    case 'L':
                        pending_payload = 1;
                        response_size = 8;
                        break;

                    case 'M':
                        // TODO
                        break;

                    case 'O':
                        pending_payload = 1;
                        response_size = 2;
                        break;

                    case 'Q':
                    case 'S':
                    case 'U':
                    case 'V':
                        pending_payload = 0;
                        _tx->write(MOCKUP_RESPONSE_BYTE);
                        break;

                    case 'T':
                        pending_payload = 8;
                        break;

                    case 'W':
                        // TODO
                        break;

                    case 'Y':
                        pending_payload = 1;
                        response_size = 1;
                        break;

                    default:
                        pending_payload = 1;
                        break;

                }

                // Show prompt
                _tx->write('>');

            } else {

                // Update pending payload
                --pending_payload;

                // If no more payload
                if (0 == pending_payload) {

                    // Inject response
                    for (uint8_t i=0; i<response_size; i++) {
                        _tx->write(MOCKUP_RESPONSE_BYTE);
                    }

                    // Reset response size
                    response_size = 0;

                    // Show prompt
                    _tx->write('>');

                }

            }
        }

        CircularBuffer * _rx;  // Chars sent to the module (using write)
        CircularBuffer * _tx;  // Chars sent by the module (read-able)

};
