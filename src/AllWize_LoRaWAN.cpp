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
 * @file AllWize_LoRaWAN.cpp
 * AllWize library LoRaWAN wrapper code file
 */

#include "AllWize_LoRaWAN.h"

/**
 * @brief           Stores the application and network keys for ABP activation
 * @param DevAddr   Device addres
 * @param AppSKey   Application Session Key
 * @param NwkSKey   Network Session Key
 * @return          Allways true since ABP joins never fail
 */
bool AllWize_LoRaWAN::joinABP(uint8_t *DevAddr, uint8_t *AppSKey, uint8_t * NwkSKey) {
    
    memcpy(_devaddr, DevAddr, 4);
    memcpy(_appskey, AppSKey, 16);
    memcpy(_nwkskey, NwkSKey, 16);
    
    #if ALLWIZE_LORAWAN_REDUCE_SIZE    
        setControlField(C_LORAWAN_UPLINK_UNCNF);
    #endif

    uint32_t uid = 0;
    for (uint8_t i=0; i<4; i++) {
        uid = (uid << 8) + DevAddr[i];
    }
    setUID(uid);

    return true;

}

/**
 * @brief               Returns latest received message (rebuilds LoRaWan header if necessary)
 * @return              New message
 */
allwize_message_t AllWize_LoRaWAN::read() {
    
    allwize_message_t raw = AllWize::read();

    // Rebuilds LoRaWAN MAC header and payload from
    // Wize header info if C-Field is 0x40
    if (C_LORAWAN_UPLINK_UNCNF == raw.c) {
        uint8_t tmp[RX_BUFFER_SIZE];
        tmp[0] = raw.c;
        tmp[1] = raw.address[3];
        tmp[2] = raw.address[2];
        tmp[3] = raw.address[1];
        tmp[4] = raw.address[0];
        tmp[5] = raw.wize_control;
        tmp[6] = raw.wize_counter & 0xFF;
        tmp[7] = raw.wize_counter >> 8;
        tmp[8] = raw.wize_application;
        memcpy(&tmp[9], raw.data, raw.len);
        raw.len += 9;
        memcpy(raw.data, tmp, raw.len);
    }

    return raw;

}

/**
 * @brief               Function to assemble and send a LoRaWAN package.
 * @param Data          Pointer to the array of data to be transmitted.
 * @param Data_Length   Length of data to be sent.
 * @param Frame_Port    Frame Port (defaults to 0x01)
 * @return              True if message was sent successfully, false otherwise
 */
bool AllWize_LoRaWAN::send(uint8_t *Data, uint8_t Data_Length, uint8_t Frame_Port) {
  
    // Define variables
    uint8_t i;
    uint8_t LoRaWAN_Data[64] = {0};
    uint8_t LoRaWAN_Data_Length = 0;
    uint8_t MIC[4] = {0};

    // Direction of frame is up
    uint8_t Direction = 0x00;

    // Unconfirmed data up
    //  [7..5] MType (010 unconfirmed up, 100 confirmed up,...)
    //  [4..2] RFU 
    //  [1..0] Major
    uint8_t Mac_Header = 0x40;

    // Frame control
    // [7] ADR
    // [6] ADRACKReq
    // [5] ACK
    // [4] ClassB
    // [3..0] FOptsLen
    uint8_t Frame_Control = 0x00;

    // Make a copy of Data
    uint8_t tmpData[Data_Length];
    for (int i = 0; i < Data_Length; i++) {
        tmpData[i] = Data[i];
    }

    // Encrypt Data (data argument is overwritten in this function)
    Encrypt_Payload(tmpData, Data_Length, _frame_counter, Direction);

    // MAC Header
    LoRaWAN_Data[0] = Mac_Header;
    
    // MAC Payload - Frame Header
    LoRaWAN_Data[1] = _devaddr[3];
    LoRaWAN_Data[2] = _devaddr[2];
    LoRaWAN_Data[3] = _devaddr[1];
    LoRaWAN_Data[4] = _devaddr[0];
    LoRaWAN_Data[5] = Frame_Control;
    LoRaWAN_Data[6] = (_frame_counter & 0x00FF);
    LoRaWAN_Data[7] = ((_frame_counter >> 8) & 0x00FF);

    // MAC Payload - Frame Port
    LoRaWAN_Data[8] = Frame_Port;

    //Set Current package length
    LoRaWAN_Data_Length = 9;

    // MAC Payload - Frame Payload
    for(i = 0; i < Data_Length; i++) {
        LoRaWAN_Data[LoRaWAN_Data_Length + i] = tmpData[i];
    }
    LoRaWAN_Data_Length += Data_Length;

    // Calculate MIC
    Calculate_MIC(LoRaWAN_Data, MIC, LoRaWAN_Data_Length, _frame_counter, Direction);

    // Load MIC in package
    for(i = 0; i < 4; i++) {
        LoRaWAN_Data[LoRaWAN_Data_Length + i] = MIC[i];
    }
    LoRaWAN_Data_Length += 4;

    #if defined(ALLWIZE_DEBUG_PORT)
        ALLWIZE_DEBUG_PORT.print("[LORAWAN] PHYPayload: ");
        char buff[6];
        for(i = 0; i < LoRaWAN_Data_Length; i++) {
            snprintf(buff, sizeof(buff), "%02X", LoRaWAN_Data[i]);
            ALLWIZE_DEBUG_PORT.print(buff);
        }
        ALLWIZE_DEBUG_PORT.println();
    #endif

    #if ALLWIZE_LORAWAN_REDUCE_SIZE    
        setCounter(_frame_counter);
        setWizeApplication(Frame_Port);
        setWizeControl(Frame_Control);
        #define ALLWIZE_LORAWAN_SKIP_BYTES 9
    #else 
        #define ALLWIZE_LORAWAN_SKIP_BYTES 0
    #endif

    // Update frame counter
    ++_frame_counter;

    // Send Package
    return AllWize::send(&LoRaWAN_Data[ALLWIZE_LORAWAN_SKIP_BYTES], LoRaWAN_Data_Length - ALLWIZE_LORAWAN_SKIP_BYTES);

}

/**
 * @brief               Returns current frame counter
 * @return              Frame counter
 */
uint16_t AllWize_LoRaWAN::getFrameCounter() { 
    return _frame_counter; 
}

/**
 * @brief               Sets new frame counter
 * @param value         2-bytes long new frame counter
 */
void AllWize_LoRaWAN::setFrameCounter(uint16_t value) { 
    _frame_counter = value; 
}

// ----------------------------------------------------------------------------

/**
 * @brief               Function used to encrypt and decrypt the data in a LoRaWAN data packet.
 * @param Data          Data Pointer to the data to decrypt or encrypt.
 * @param Data_Length   Number of bytes to be transmitted.
 * @param Frame_Counter Frame_Counter. Counts upstream frames.
 * @param Direction     Direction of message (is up).
 * @private
 */
void AllWize_LoRaWAN::Encrypt_Payload(uint8_t *Data, uint8_t Data_Length, uint16_t Frame_Counter, uint8_t Direction) {
    
    uint8_t i = 0x00;
    uint8_t j;
    uint8_t Number_of_Blocks = 0x00;
    uint8_t Incomplete_Block_Size = 0x00;

    uint8_t Block_A[16];

    //Calculate number of blocks
    Number_of_Blocks = Data_Length / 16;
    Incomplete_Block_Size = Data_Length % 16;
    if (Incomplete_Block_Size != 0) {
        Number_of_Blocks++;
    }

    for (i = 1; i <= Number_of_Blocks; i++) {

        Block_A[0] = 0x01;
        Block_A[1] = 0x00;
        Block_A[2] = 0x00;
        Block_A[3] = 0x00;
        Block_A[4] = 0x00;

        Block_A[5] = Direction;

        Block_A[6] = _devaddr[3];
        Block_A[7] = _devaddr[2];
        Block_A[8] = _devaddr[1];
        Block_A[9] = _devaddr[0];

        Block_A[10] = (Frame_Counter & 0x00FF);
        Block_A[11] = ((Frame_Counter >> 8) & 0x00FF);

        Block_A[12] = 0x00; //Frame counter upper Bytes
        Block_A[13] = 0x00;

        Block_A[14] = 0x00;

        Block_A[15] = i;

        //Calculate S
        AES_Encrypt(Block_A, _appskey); //original

        //Check for last block
        if (i != Number_of_Blocks) {
            for (j = 0; j < 16; j++) {
                *Data = *Data ^ Block_A[j];
                Data++;
            }
        } else {
            if (Incomplete_Block_Size == 0) {
                Incomplete_Block_Size = 16;
            }
            for (j = 0; j < Incomplete_Block_Size; j++) {
                *Data = *Data ^ Block_A[j];
                Data++;
            }
        }
    }
}

/**
 * @brief               Function used to calculate the validity of data messages.
 * @param Data          Data Pointer to the data to decrypt or encrypt.
 * @param Final_MIC     Pointer to MIC array (4 bytes).
 * @param Data_Length   Number of bytes to be transmitted.
 * @param Frame_Counter Frame counter of upstream frames.
 * @param Direction     Direction of message (is up?).
 * @private
*/
void AllWize_LoRaWAN::Calculate_MIC(uint8_t *Data, uint8_t *Final_MIC, uint8_t Data_Length, uint16_t Frame_Counter, uint8_t Direction) {
    
    uint8_t i;
    uint8_t Block_B[16];

    uint8_t Key_K1[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t Key_K2[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    //uint8_t Data_Copy[16];

    uint8_t Old_Data[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    uint8_t New_Data[16] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    uint8_t Number_of_Blocks = 0x00;
    uint8_t Incomplete_Block_Size = 0x00;
    uint8_t Block_Counter = 0x01;

    //Create Block_B
    Block_B[0] = 0x49;
    Block_B[1] = 0x00;
    Block_B[2] = 0x00;
    Block_B[3] = 0x00;
    Block_B[4] = 0x00;

    Block_B[5] = Direction;

    Block_B[6] = _devaddr[3];
    Block_B[7] = _devaddr[2];
    Block_B[8] = _devaddr[1];
    Block_B[9] = _devaddr[0];

    Block_B[10] = (Frame_Counter & 0x00FF);
    Block_B[11] = ((Frame_Counter >> 8) & 0x00FF);

    Block_B[12] = 0x00; //Frame counter upper bytes
    Block_B[13] = 0x00;

    Block_B[14] = 0x00;
    Block_B[15] = Data_Length;

    //Calculate number of Blocks and blocksize of last block
    Number_of_Blocks = Data_Length / 16;
    Incomplete_Block_Size = Data_Length % 16;

    if (Incomplete_Block_Size != 0) {
        Number_of_Blocks++;
    }

    Generate_Keys(Key_K1, Key_K2);

    //Preform Calculation on Block B0

    //Preform AES encryption
    AES_Encrypt(Block_B, _nwkskey);

    //Copy Block_B to Old_Data
    for (i = 0; i < 16; i++) {
        Old_Data[i] = Block_B[i];
    }

    //Preform full calculating until n-1 messsage blocks
    while (Block_Counter < Number_of_Blocks) {

        //Copy data into array
        for (i = 0; i < 16; i++) {
            New_Data[i] = *Data;
            Data++;
        }

        //Preform XOR with old data
        XOR(New_Data, Old_Data);

        //Preform AES encryption
        AES_Encrypt(New_Data, _nwkskey);

        //Copy New_Data to Old_Data
        for (i = 0; i < 16; i++) {
            Old_Data[i] = New_Data[i];
        }

        //Raise Block counter
        Block_Counter++;

    }

    //Perform calculation on last block
    //Check if Datalength is a multiple of 16
    if (Incomplete_Block_Size == 0) {

        //Copy last data into array
        for (i = 0; i < 16; i++) {
            New_Data[i] = *Data;
            Data++;
        }

        //Preform XOR with Key 1
        XOR(New_Data, Key_K1);

        //Preform XOR with old data
        XOR(New_Data, Old_Data);

        //Preform last AES routine
        // read _nwkskey from PROGMEM
        AES_Encrypt(New_Data, _nwkskey);
    
    } else {

        //Copy the remaining data and fill the rest
        for (i = 0; i < 16; i++) {
            if (i < Incomplete_Block_Size) {
                New_Data[i] = *Data;
                Data++;
            }
            if (i == Incomplete_Block_Size) {
                New_Data[i] = 0x80;
            }
            if (i > Incomplete_Block_Size) {
                New_Data[i] = 0x00;
            }
        }

        //Preform XOR with Key 2
        XOR(New_Data, Key_K2);

        //Preform XOR with Old data
        XOR(New_Data, Old_Data);

        //Preform last AES routine
        AES_Encrypt(New_Data, _nwkskey);
    
    }

    Final_MIC[0] = New_Data[0];
    Final_MIC[1] = New_Data[1];
    Final_MIC[2] = New_Data[2];
    Final_MIC[3] = New_Data[3];

}

/**
 * @brief       Function used to generate keys for the MIC calculation.
 * @param K1    Pointer to Key1.
 * @param K2    Pointer to Key2.
 * @private
 */
void AllWize_LoRaWAN::Generate_Keys(uint8_t *K1, uint8_t *K2) {

    uint8_t i;
    uint8_t MSB_Key;

    //Encrypt the zeros in K1 with the _nwkskey
    AES_Encrypt(K1, _nwkskey);

    //Create K1
    //Check if MSB is 1
    if ((K1[0] & 0x80) == 0x80) {
        MSB_Key = 1;
    } else {
        MSB_Key = 0;
    }

    //Shift K1 one bit left
    Shift_Left(K1);

    //if MSB was 1
    if (MSB_Key == 1) {
        K1[15] = K1[15] ^ 0x87;
    }

    //Copy K1 to K2
    for (i = 0; i < 16; i++) {
        K2[i] = K1[i];
    }

    //Check if MSB is 1
    if ((K2[0] & 0x80) == 0x80) {
        MSB_Key = 1;
    } else {
        MSB_Key = 0;
    }

    //Shift K2 one bit left
    Shift_Left(K2);

    //Check if MSB was 1
    if (MSB_Key == 1) {
        K2[15] = K2[15] ^ 0x87;
    }

}

/**
 * @brief       Round-shifts data to the left
 * @param Data  Data buffer
 * @private
 */
void AllWize_LoRaWAN::Shift_Left(uint8_t *Data) {

    uint8_t i;
    uint8_t Overflow = 0;
    //uint8_t High_Byte, Low_Byte;

    for (i = 0; i < 16; i++) {

        //Check for overflow on next byte except for the last byte
        if (i < 15) {
            //Check if upper bit is one
            if ((Data[i + 1] & 0x80) == 0x80) {
                Overflow = 1;
            } else {
                Overflow = 0;
            }
        } else {
            Overflow = 0;
        }

        //Shift one left
        Data[i] = (Data[i] << 1) + Overflow;

    }

}

/**
 * @brief           Function to XOR two character arrays.
 * @param New_Data  A pointer to the calculated data.
 * @param Old_Data  A pointer to the data to be xor'd.
 * @private
 */
void AllWize_LoRaWAN::XOR(uint8_t *New_Data, uint8_t *Old_Data) {

    uint8_t i;
    for (i = 0; i < 16; i++) {
        New_Data[i] = New_Data[i] ^ Old_Data[i];
    }

}

//-----------------------------------------------------------------------------
// AES Encryption methods
//-----------------------------------------------------------------------------

/*
 * Description: S_Table used for AES encription
 */
const uint8_t PROGMEM AllWize_LoRaWAN::S_Table[16][16] = {
	  {0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76},
	  {0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0},
	  {0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15},
	  {0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75},
	  {0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84},
	  {0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF},
	  {0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8},
	  {0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2},
	  {0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73},
	  {0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB},
	  {0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79},
	  {0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08},
	  {0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A},
	  {0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E},
	  {0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF},
	  {0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16}
	};

/**
 * @brief       Function used to perform AES encryption.
 * @param Data  Pointer to the data to decrypt or encrypt.
 * @param Key   Pointer to AES encryption key.
 * @private
 */
void AllWize_LoRaWAN::AES_Encrypt(uint8_t *Data, const uint8_t *Key) {
    
    uint8_t Row, Column, Round = 0;
    uint8_t Round_Key[16];
    uint8_t State[4][4];

    //  Copy input to State arry
    for (Column = 0; Column < 4; Column++) {
        for (Row = 0; Row < 4; Row++) {
            State[Row][Column] = Data[Row + (Column << 2)];
        }
    }

    //  Copy key to round key
    memcpy(&Round_Key[0], &Key[0], 16);

    //  Add round key
    AES_Add_Round_Key(Round_Key, State);

    //  Preform 9 full rounds with mixed collums
    for (Round = 1; Round < 10; Round++) {

        //  Perform Byte substitution with S table
        for (Column = 0; Column < 4; Column++) {
            for (Row = 0; Row < 4; Row++) {
                State[Row][Column] = AES_Sub_Byte(State[Row][Column]);
            }
        }

        //  Perform Row Shift
        AES_Shift_Rows(State);

        //  Mix Collums
        AES_Mix_Collums(State);

        //  Calculate new round key
        AES_Calculate_Round_Key(Round, Round_Key);

        //  Add the round key to the Round_key
        AES_Add_Round_Key(Round_Key, State);

    }

    //  Perform Byte substitution with S table whitout mix collums
    for (Column = 0; Column < 4; Column++) {
        for (Row = 0; Row < 4; Row++) {
            State[Row][Column] = AES_Sub_Byte(State[Row][Column]);
        }
    }

    //  Shift rows
    AES_Shift_Rows(State);

    //  Calculate new round key
    AES_Calculate_Round_Key(Round, Round_Key);

    //  Add round key
    AES_Add_Round_Key(Round_Key, State);

    //  Copy the State into the data array
    for (Column = 0; Column < 4; Column++) {
        for (Row = 0; Row < 4; Row++) {
            Data[Row + (Column << 2)] = State[Row][Column];
        }
    }

}

/**
 * @brief           Function performs AES AddRoundKey step.
 * @param Round_Key Pointer to the round subkey.
 * @param *State    Pointer to bytes of the states-to-be-xor'd.
 * @private
 */
void AllWize_LoRaWAN::AES_Add_Round_Key(uint8_t *Round_Key, uint8_t (*State)[4]) {
    
    uint8_t Row, Collum;

    for (Collum = 0; Collum < 4; Collum++) {
        for (Row = 0; Row < 4; Row++) {
            State[Row][Collum] ^= Round_Key[Row + (Collum << 2)];
        }
    }

}

/**
 * @brief       Function performs AES SubBytes step.
 * @param Byte  Individual byte, from state array.
 * @return      Byte from the S_Table table
 * @private
 */
uint8_t AllWize_LoRaWAN::AES_Sub_Byte(uint8_t Byte) {

    //  uint8_t S_Row,S_Collum;
    //  uint8_t S_Byte;
    //
    //  S_Row    = ((Byte >> 4) & 0x0F);
    //  S_Collum = ((Byte >> 0) & 0x0F);
    //  S_Byte   = S_Table [S_Row][S_Collum];

    //return S_Table [ ((Byte >> 4) & 0x0F) ] [ ((Byte >> 0) & 0x0F) ]; // original
    return pgm_read_byte(&(S_Table[((Byte >> 4) & 0x0F)][((Byte >> 0) & 0x0F)]));

}

/**
 * @brief           Function performs AES ShiftRows step.
 * @param *State    Pointer to state array.
 * @private
 */
void AllWize_LoRaWAN::AES_Shift_Rows(uint8_t (*State)[4]) {

    uint8_t Buffer;

    //Store firt byte in buffer
    Buffer = State[1][0];
    //Shift all bytes
    State[1][0] = State[1][1];
    State[1][1] = State[1][2];
    State[1][2] = State[1][3];
    State[1][3] = Buffer;

    Buffer = State[2][0];
    State[2][0] = State[2][2];
    State[2][2] = Buffer;
    Buffer = State[2][1];
    State[2][1] = State[2][3];
    State[2][3] = Buffer;

    Buffer = State[3][3];
    State[3][3] = State[3][2];
    State[3][2] = State[3][1];
    State[3][1] = State[3][0];
    State[3][0] = Buffer;

}

/**
 * @brief           Function performs AES MixColumns step.
 * @param *State    Pointer to state array.
 * @private
 */
void AllWize_LoRaWAN::AES_Mix_Collums(uint8_t (*State)[4]) {
    
    uint8_t Row, Collum;
    uint8_t a[4], b[4];

    for (Collum = 0; Collum < 4; Collum++) {
        for (Row = 0; Row < 4; Row++) {
            a[Row] = State[Row][Collum];
            b[Row] = (State[Row][Collum] << 1);

            if ((State[Row][Collum] & 0x80) == 0x80) {
                b[Row] ^= 0x1B;
            }
        }

        State[0][Collum] = b[0] ^ a[1] ^ b[1] ^ a[2] ^ a[3];
        State[1][Collum] = a[0] ^ b[1] ^ a[2] ^ b[2] ^ a[3];
        State[2][Collum] = a[0] ^ a[1] ^ b[2] ^ a[3] ^ b[3];
        State[3][Collum] = a[0] ^ b[0] ^ a[1] ^ a[2] ^ b[3];
    }

}

/**
 * @brief           Function performs AES Round Key Calculation.
 * @param Round     Number of rounds to perform (depends on key size).
 * @param Round_Key Pointer to round key.
 * @private
 */
void AllWize_LoRaWAN::AES_Calculate_Round_Key(uint8_t Round, uint8_t *Round_Key) {
    
    uint8_t i, j, b, Rcon;
    uint8_t Temp[4];

    //Calculate Rcon
    Rcon = 0x01;
    while (Round != 1) {
        b = Rcon & 0x80;
        Rcon = Rcon << 1;

        if (b == 0x80) {
            Rcon ^= 0x1b;
        }
        Round--;
    }

    //  Calculate first Temp
    //  Copy laste byte from previous key and subsitute the byte, but shift the array contents around by 1.
    Temp[0] = AES_Sub_Byte(Round_Key[12 + 1]);
    Temp[1] = AES_Sub_Byte(Round_Key[12 + 2]);
    Temp[2] = AES_Sub_Byte(Round_Key[12 + 3]);
    Temp[3] = AES_Sub_Byte(Round_Key[12 + 0]);

    //  XOR with Rcon
    Temp[0] ^= Rcon;

    //  Calculate new key
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            Round_Key[j + (i << 2)] ^= Temp[j];
            Temp[j] = Round_Key[j + (i << 2)];
        }
    }

}