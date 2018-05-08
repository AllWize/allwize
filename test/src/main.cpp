/*

Allwize - Test suite

This test suite uses Aunit unit testing framework (https://github.com/bxparks/AUnit)
and RC1701XX_Mockup class that mocks up the RC1701XX radio module

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
#include "RC1701XX_Mockup.h"

#include "AUnit.h"
using namespace aunit;

// -----------------------------------------------------------------------------
// Test class
// -----------------------------------------------------------------------------

class CustomTest: public TestOnce {

    protected:

        virtual void setup() override {
            mock = new RC1701XX_Mockup();
            allwize = new Allwize(*mock);
            mock->reset();
        }

        virtual void teardown() override {
            delete allwize;
            delete mock;
        }

        virtual void compare(size_t len, uint8_t * expected) {
            assertEqual(len, (size_t) mock->rx_available());
            for (uint8_t i=0; i<len; i++) {
                assertEqual(mock->rx_read(), expected[i]);
            }
        }

        RC1701XX_Mockup * mock;
        Allwize * allwize;

};

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

testF(CustomTest, reset) {
    allwize->reset();
    uint8_t expected[] = {0x00, 'M', 0x37, 1, 0xFF, 0x58, 0x00, '@', 'R', 'R'};
    compare(sizeof(expected), expected);
}

testF(CustomTest, set_channel) {
    uint8_t channel = 3;
    allwize->setChannel(channel);
    uint8_t expected[] = {0x00, 'C', channel, 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, set_channel_persist) {
    uint8_t channel = 3;
    allwize->setChannel(channel, true);
    uint8_t expected[] = {0x00, 'M', 0x00, channel, 0xFF, 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, get_channel) {
    allwize->getChannel();
    uint8_t expected[] = {0x00, 'Y', 0x00, 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, set_mbus_mode) {
    uint8_t mode = MBUS_MODE_OSP;
    allwize->setMBusMode(mode);
    uint8_t expected[] = {0x00, 'G', mode, 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, set_mbus_mode_persist) {
    uint8_t mode = MBUS_MODE_OSP;
    allwize->setMBusMode(mode, true);
    uint8_t expected[] = {0x00, 'M', 0x03, mode, 0xFF, 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, get_mbus_mode) {
    allwize->getMBusMode();
    uint8_t expected[] = {0x00, 'Y', 0x03, 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, set_install) {
    uint8_t mode = INSTALL_MODE_NORMAL;
    allwize->setInstallMode(mode);
    uint8_t expected[] = {0x00, 'I', mode, 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, set_control_field) {
    uint8_t value = 0x06;
    allwize->setControlField(value);
    uint8_t expected[] = {0x00, 'F', value, 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, set_control_field_persist) {
    uint8_t value = 0x06;
    allwize->setControlField(value, true);
    uint8_t expected[] = {0x00, 'M', 0x3B, value, 0xFF, 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, get_control_field) {
    allwize->getControlField();
    uint8_t expected[] = {0x00, 'Y', 0x3B, 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, set_default_key) {
    uint8_t key[16];
    for (uint8_t i=0; i<16; i++) key[i] = 0x10;
    allwize->setDefaultKey(key);
    uint8_t expected[] = {
        0x00, 'M',
        0x40, 0x10, 0x41, 0x10, 0x42, 0x10, 0x43, 0x10,
        0x44, 0x10, 0x45, 0x10, 0x46, 0x10, 0x47, 0x10,
        0x48, 0x10, 0x49, 0x10, 0x4A, 0x10, 0x4B, 0x10,
        0x4C, 0x10, 0x4D, 0x10, 0x4E, 0x10, 0x4F, 0x10,
        0xFF, 0x58
    };
    compare(sizeof(expected), expected);
}

testF(CustomTest, get_temperature) {
    uint8_t value = allwize->getTemperature();
    if (32 != value) return false;
    uint8_t expected[] = {0x00, 'U', 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, get_voltage) {
    uint16_t value = allwize->getVoltage();
    if (4800 != value) return false;
    uint8_t expected[] = {0x00, 'V', 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, get_rssi) {
    float value = allwize->getRSSI();
    if (-80 != value) return false;
    uint8_t expected[] = {0x00, 'S', 0x58};
    compare(sizeof(expected), expected);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    Serial.begin(115200);
    while (!Serial);

    Serial.println();
    Serial.println("Allwize library test suite");
    Serial.println();

    //TestRunner::setVerbosity(Verbosity::kAll);

}

void loop() {
    TestRunner::run();
}
