/*

AllWize LoRaWAN Library

This code is based on Adafruit's TinyLora Library and thus

Copyright (C) 2015, 2016 Ideetron B.V.
Modified by Brent Rubell for Adafruit Industries.
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
 * @file AllWize_LoRaWAN.h
 * AllWize library LoRaWAN wrapper header file
 */

#pragma once

#include <Arduino.h>
#include "AllWize.h"

// Setting ALLWIZE_LORAWAN_REDUCE_SIZE to 1 reduced the payload by 9 bytes 
// by merging the LoRaWAN MAC header and payload and WIZE headers
// The message is then sent using special C-Field 0x40
#ifndef ALLWIZE_LORAWAN_REDUCE_SIZE
#define ALLWIZE_LORAWAN_REDUCE_SIZE 1
#endif

class AllWize_LoRaWAN: public AllWize {

    public:
        
        AllWize_LoRaWAN(HardwareSerial * serial, uint8_t reset_gpio = GPIO_NONE, uint8_t config_gpio = GPIO_NONE): AllWize(serial, reset_gpio, config_gpio) {}
        #if not defined(ARDUINO_ARCH_SAMD) && not defined(ARDUINO_ARCH_ESP32)
        AllWize_LoRaWAN(SoftwareSerial * serial, uint8_t reset_gpio = GPIO_NONE, uint8_t config_gpio = GPIO_NONE): AllWize(serial, reset_gpio, config_gpio) {}
        #endif
        AllWize_LoRaWAN(uint8_t rx, uint8_t tx, uint8_t reset_gpio = GPIO_NONE, uint8_t config_gpio = GPIO_NONE): AllWize(rx, tx, reset_gpio, config_gpio) {}

        allwize_message_t read();
        bool joinABP(uint8_t *DevAddr, uint8_t *AppSKey, uint8_t * NwkSKey);
        bool send(uint8_t *Data, uint8_t Data_Length, uint8_t Frame_Port = 0x01);
        uint16_t getFrameCounter();
        void setFrameCounter(uint16_t value);

    private:

        uint8_t _devaddr[4];
        uint8_t _appskey[16];
        uint8_t _nwkskey[16];
        uint16_t _frame_counter = 0;
        static const uint8_t S_Table[16][16];

        void Encrypt_Payload(uint8_t *Data, uint8_t Data_Length, uint16_t Frame_Counter, uint8_t Direction);
        void Calculate_MIC(uint8_t *Data, uint8_t *Final_MIC, uint8_t Data_Length, uint16_t Frame_Counter, uint8_t Direction);
        void Generate_Keys(uint8_t *K1, uint8_t *K2);
        void Shift_Left(uint8_t *Data);
        void XOR(uint8_t *New_Data, uint8_t *Old_Data);

        void AES_Encrypt(uint8_t *Data, const uint8_t *Key);
        void AES_Add_Round_Key(uint8_t *Round_Key, uint8_t(*State)[4]);
        uint8_t AES_Sub_Byte(uint8_t Byte);
        void AES_Shift_Rows(uint8_t(*State)[4]);
        void AES_Mix_Collums(uint8_t(*State)[4]);
        void AES_Calculate_Round_Key(uint8_t Round, uint8_t *Round_Key);

};
