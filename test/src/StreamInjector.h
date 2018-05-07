/*

StreamInjector

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

class StreamInjector : public Stream {

    public:

        typedef void (*writeCallback)(uint8_t);

        StreamInjector(uint8_t tx_buffer_size = 128) : _tx_buffer_size(tx_buffer_size) {
            _tx_buffer = new char[tx_buffer_size];
        }

        ~StreamInjector() {
            delete[] _tx_buffer;
        }

        // ---------------------------------------------------------------------

        virtual uint16_t inject(char ch) {
            _tx_buffer[_tx_buffer_write] = ch;
            _tx_buffer_write = (_tx_buffer_write + 1) % _tx_buffer_size;
            return 1;
        }

        virtual uint16_t inject(char *data, uint16_t len) {
            for (uint16_t i=0; i<len; i++) {
                inject(data[i]);
            }
            return len;
        }

        virtual void callback(writeCallback c) {
            _callback = c;
        }

        // ---------------------------------------------------------------------

        virtual size_t write(uint8_t ch) {
            if (_callback) _callback(ch);
            return 1;
        }

        virtual int read() {
            int ch = -1;
            if (_tx_buffer_read != _tx_buffer_write) {
                ch = (uint8_t) _tx_buffer[_tx_buffer_read];
                _tx_buffer_read = (_tx_buffer_read + 1) % _tx_buffer_size;
            }
            return ch;
        }

        virtual int available() {
            int bytes = 0;
            if (_tx_buffer_read > _tx_buffer_write) {
                bytes += (_tx_buffer_write - _tx_buffer_read + _tx_buffer_size);
            } else if (_tx_buffer_read < _tx_buffer_write) {
                bytes += (_tx_buffer_write - _tx_buffer_read);
            }
            return bytes;
        }

        virtual int peek() {
            int ch = -1;
            if (_tx_buffer_read != _tx_buffer_write) {
                ch = _tx_buffer[_tx_buffer_read];
            }
            return ch;
        }

        virtual void flush() {
            _tx_buffer_read = _tx_buffer_write;
        }

    private:

        char * _tx_buffer;
        uint16_t _tx_buffer_size;
        uint16_t _tx_buffer_write = 0;
        uint16_t _tx_buffer_read = 0;
        writeCallback _callback = NULL;

};
