/*

Serial Sniffer

Copyright (C) 2018 by Xose Pérez <xose dot perez at gmail dot com>

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

class SerialSniffer : public Stream {

    public:

        SerialSniffer(Stream & stream, Stream & debug) : _stream(stream), _debug(debug) {}

        virtual size_t write(uint8_t ch) {
            _debug.print("w ");
            _debug.print(ch, HEX);
            _debug.println();
            delay(1);
            return _stream.write(ch);
        }

        virtual int read() {
            int ch = _stream.read();
            if (ch >= 0) {
                _debug.print("r ");
                _debug.print(ch, HEX);
                _debug.println();
                delay(1);
            }
            return ch;
        }

        virtual int available() {
            return _stream.available();
        }

        virtual int peek() {
            return _stream.peek();
        }

        virtual void flush() {
            _stream.flush();
        }

    private:

        Stream & _stream;
        Stream & _debug;


};
