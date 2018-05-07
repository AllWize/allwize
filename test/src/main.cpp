#include "Allwize.h"
#include "StreamInjector.h"

StreamInjector mock;
Allwize * allwize;

#define TEST_BUFFER_SIZE    32
#define TEST_NAME_PADDING   30
#define TEST_RESPONSE_BYTE  0xA0
#define TEST_DEBUG          0

// -----------------------------------------------------------------------------

uint8_t _rx_buffer[TEST_BUFFER_SIZE];
uint8_t _rx_read = 0;
uint8_t _rx_write = 0;
uint8_t _tests = 0;
uint8_t _tests_failed = 0;

void _reset() {
    _rx_write = _rx_read = 0;
}

uint8_t available() {
    if (_rx_write >= _rx_read) return _rx_write - _rx_read;
    return TEST_BUFFER_SIZE - _rx_read + _rx_write;
}

bool _compare(uint8_t * expected, size_t len) {

    bool response = true;

    Serial.print(len); Serial.print(available());
    if (available() != len) response = false;
    if (available() != len) Serial.println("does not match");

    for (uint8_t i=0; i<len; i++) {
        if (_rx_buffer[_rx_read] != expected[i]) response = false;
        _rx_read = (_rx_read + 1) % TEST_BUFFER_SIZE;
    }
    return response;
}

void _mock_inject(unsigned char ch) {
    #if TEST_DEBUG
        Serial.print("Receive: 0x");
        Serial.println(ch, HEX);
    #endif
    mock.inject(ch);
}

void _mock_response(unsigned char ch) {

    static uint8_t pending_payload = 0;
    static uint8_t response_size = 0;
    static bool config_mode = false;

    #if TEST_DEBUG
        Serial.print("Sent   : 0x");
        Serial.println(ch, HEX);
    #endif

    // Buffer
    _rx_buffer[_rx_write] = ch;
    _rx_write = (_rx_write + 1) % TEST_BUFFER_SIZE;

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
                _mock_inject(TEST_RESPONSE_BYTE);
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
        _mock_inject('>');

    } else {

        // Update pending payload
        --pending_payload;

        // If no more payload
        if (0 == pending_payload) {

            // Inject response
            for (uint8_t i=0; i<response_size; i++) {
                _mock_inject(TEST_RESPONSE_BYTE);
            }

            // Reset response size
            response_size = 0;

            // Show prompt
            _mock_inject('>');

        }

    }

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

    _reset();
    mock.flush();

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

    mock.callback(_mock_response);
    allwize = new Allwize(mock);

    tests();

    Serial.println();
    Serial.print(_tests_failed);
    Serial.print("/");
    Serial.print(_tests);
    Serial.println(" tests failed");

}

void loop() {}
