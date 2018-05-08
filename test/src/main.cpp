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
    uint8_t value = 0x10;
    uint8_t key[16];
    for (uint8_t i=0; i<sizeof(key); i++) key[i] = value;
    allwize->setDefaultKey(key);
    uint8_t expected[] = {
        0x00, 'M',
        0x40, value, 0x41, value, 0x42, value, 0x43, value,
        0x44, value, 0x45, value, 0x46, value, 0x47, value,
        0x48, value, 0x49, value, 0x4A, value, 0x4B, value,
        0x4C, value, 0x4D, value, 0x4E, value, 0x4F, value,
        0xFF, 0x58
    };
    compare(sizeof(expected), expected);
}

testF(CustomTest, get_temperature) {
    assertEqual(32, (int) allwize->getTemperature());
    uint8_t expected[] = {0x00, 'U', 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, get_voltage) {
    assertEqual(4800, (int) allwize->getVoltage());
    uint8_t expected[] = {0x00, 'V', 0x58};
    compare(sizeof(expected), expected);
}

testF(CustomTest, get_rssi) {
    assertEqual(-80, (int) allwize->getRSSI());
    uint8_t expected[] = {0x00, 'S', 0x58};
    compare(sizeof(expected), expected);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    Serial.begin(115200);
    while (!Serial);

    //TestRunner::setVerbosity(Verbosity::kAll);

}

void loop() {
    TestRunner::run();
}
