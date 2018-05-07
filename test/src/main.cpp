#include "Allwize.h"
#include "RC1701XX_Mockup.h"

RC1701XX_Mockup mock;
Allwize * allwize;

#define TEST_NAME_PADDING   30
#define TEST_RESPONSE_BYTE  0xA0

// -----------------------------------------------------------------------------

uint8_t _tests = 0;
uint8_t _tests_failed = 0;

bool _compare(uint8_t * expected, size_t len) {

    if ((size_t) mock.rx_available() != len) return false;

    for (uint8_t i=0; i<len; i++) {
        uint8_t ch = mock.rx_read();
        if (ch != expected[i]) return false;
    }
    return true;
}

// -----------------------------------------------------------------------------

bool test_set_channel(void) {
    uint8_t channel = 3;
    allwize->setChannel(channel);
    uint8_t expected[] = {0x00, 'C', channel, 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_set_channel_persist(void) {
    uint8_t channel = 3;
    allwize->setChannel(channel, true);
    uint8_t expected[] = {0x00, 'M', 0x00, channel, 0xFF, 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_get_channel(void) {
    allwize->getChannel();
    uint8_t expected[] = {0x00, 'Y', 0x00, 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_set_mbus_mode(void) {
    uint8_t mode = MBUS_MODE_R2;
    allwize->setMBusMode(mode);
    uint8_t expected[] = {0x00, 'G', mode, 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_set_mbus_mode_persist(void) {
    uint8_t mode = MBUS_MODE_R2;
    allwize->setMBusMode(mode, true);
    uint8_t expected[] = {0x00, 'M', 0x03, mode, 0xFF, 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_get_mbus_mode(void) {
    allwize->getMBusMode();
    uint8_t expected[] = {0x00, 'Y', 0x03, 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_set_install(void) {
    uint8_t mode = INSTALL_MODE_NORMAL;
    allwize->setInstallMode(mode);
    uint8_t expected[] = {0x00, 'I', mode, 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_set_control_field(void) {
    uint8_t value = 0x06;
    allwize->setControlField(value);
    uint8_t expected[] = {0x00, 'F', value, 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_set_control_field_persist(void) {
    uint8_t value = 0x06;
    allwize->setControlField(value, true);
    uint8_t expected[] = {0x00, 'M', 0x3B, value, 0xFF, 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_get_control_field(void) {
    allwize->getControlField();
    uint8_t expected[] = {0x00, 'Y', 0x3B, 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_get_temperature(void) {
    uint8_t value = allwize->getTemperature();
    if (32 != value) return false;
    uint8_t expected[] = {0x00, 'U', 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_get_voltage(void) {
    uint16_t value = allwize->getVoltage();
    if (4800 != value) return false;
    uint8_t expected[] = {0x00, 'V', 0x58};
    return _compare(expected, sizeof(expected));
}

bool test_get_rssi(void) {
    float value = allwize->getRSSI();
    if (-80 != value) return false;
    uint8_t expected[] = {0x00, 'S', 0x58};
    return _compare(expected, sizeof(expected));
}

// -----------------------------------------------------------------------------

void test(const char * name, bool (*callback)(void)) {

    mock.flush();
    mock.rx_flush();

    bool response = (callback)();

    Serial.print(name);
    for (uint8_t i=strlen(name); i<TEST_NAME_PADDING; i++) Serial.print(".");
    Serial.println(response ? "OK" : "FAIL");
    ++_tests;
    if (!response) ++_tests_failed;

}

void tests() {

    test("setChannel", test_set_channel);
    test("setChannelPersist", test_set_channel_persist);
    test("getChannel", test_get_channel);

    test("setControlField", test_set_control_field);
    test("setControlFieldPersist", test_set_control_field_persist);
    test("getControlField", test_get_control_field);

    test("setMBusMode", test_set_mbus_mode);
    test("setMBusModePersist", test_set_mbus_mode_persist);
    test("getMBusMode", test_get_mbus_mode);

    test("setInstallMode", test_set_install);
    test("getTemperature", test_get_temperature);
    test("getVoltage", test_get_voltage);
    test("getRSSI", test_get_rssi);

}

// -----------------------------------------------------------------------------

void setup() {

    Serial.begin(115200);
    while (!Serial);
    Serial.println();
    Serial.println("Allwize library test suite");
    for (uint8_t i=0; i<TEST_NAME_PADDING+4; i++) Serial.print("-");
    Serial.println();

    allwize = new Allwize(mock);

    tests();

    Serial.println();
    Serial.print(_tests_failed);
    Serial.print("/");
    Serial.print(_tests);
    Serial.println(" tests failed");

}

void loop() {}
