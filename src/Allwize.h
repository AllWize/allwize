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

#ifndef ALLWIZE_H
#define ALLWIZE_H

#include <Arduino.h>
#include <Stream.h>

#define MODEM_BAUDRATE          19200
#define END_OF_RESPONSE         '>'
#define CMD_ENTER_CONFIG        (char) 0x00
#define CMD_EXIT_CONFIG         (char) 0xFF
#define CMD_MEMORY              'M'
#define CMD_CHANNEL             'C'
#define RX_BUFFER_SIZE          32

class Allwize {

    public:

        Allwize(Stream& stream, uint8_t reset_gpio = 0xFF);

        void begin();
        void reset();
        //void setTimeout(uint32_t timeout);

        void setChannel(uint8_t channel);

    protected:

        bool _setConfig(bool value);
        void _sendCommand(uint8_t command, uint8_t * data, size_t len);
        void _sendCommand(uint8_t command, uint8_t data);
        void _setMemory(uint8_t address, uint8_t * data, size_t len);
        void _setMemory(uint8_t address, uint8_t data);
        //size_t _getMemory(uint8_t address, size_t len, uint8_t * buffer);
        //uint8_t _getMemory(uint8_t address);

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

        uint8_t * _buffer[RX_BUFFER_SIZE];

};

#endif // ALLWIZE_H
