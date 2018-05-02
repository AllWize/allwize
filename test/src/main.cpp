#include "Allwize.h"
#include "StreamInjector.h"

StreamInjector mock;
Allwize * allwize;

#define TEST_NAME_PADDING   30
#define MOCK_RESPONSE_BYTE  0x80

// -----------------------------------------------------------------------------

uint8_t _rx_buffer[32];
uint8_t _rx_count = 0;

void _reset() {
    _rx_count = 0;
}

bool _compare(uint8_t * expected, size_t len) {
    if (len != _rx_count) return false;
    for (uint8_t i=0; i<len; i++) {
        if (_rx_buffer[i] != expected[i]) return false;
    }
    return true;
}

void _mock_response(unsigned char ch) {

    static uint8_t pending_payload = 0;
    static uint8_t response_size = 0;

    _rx_buffer[_rx_count++] = ch;

    // Expected payload sizes (defaults to 1 byte)
    if (0 == pending_payload) {

        // Show prompt
        mock.inject('>');

        // Handle cases
        switch (ch) {

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
                mock.inject(MOCK_RESPONSE_BYTE);
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

    } else {

        // Update pending payload
        --pending_payload;

        // If no more payload
        if (0 == pending_payload) {

            // Inject response
            for (uint8_t i=0; i<response_size; i++) {
                mock.inject(MOCK_RESPONSE_BYTE);
            }

            // Reset response size
            response_size = 0;

            // Show prompt
            mock.inject('>');

        }

    }

}

// -----------------------------------------------------------------------------

bool test_set_channel(void) {
    uint8_t channel = 3;
    allwize->setChannel(channel);
    uint8_t expected[] = {0x00, 'C', channel, 0x58};
    return _compare(expected, 4);
}

bool test_set_channel_persist(void) {
    uint8_t channel = 3;
    allwize->setChannel(channel, true);
    uint8_t expected[] = {0x00, 'M', 0x00, channel, 0xFF, 0x58};
    return _compare(expected, 6);
}

bool test_set_mbus_mode(void) {
    allwize_mbus_mode_t mode = MBUS_MODE_R2;
    allwize->setMBusMode(mode);
    uint8_t expected[] = {0x00, 'G', mode, 0x58};
    return _compare(expected, 4);
}

bool test_set_mbus_mode_persist(void) {
    allwize_mbus_mode_t mode = MBUS_MODE_R2;
    allwize->setMBusMode(mode, true);
    uint8_t expected[] = {0x00, 'M', 0x03, mode, 0xFF, 0x58};
    return _compare(expected, 6);
}

bool test_set_op_mode(void) {
    allwize_operation_mode_t mode = OP_MODE_NORMAL;
    allwize->setOperationMode(mode);
    uint8_t expected[] = {0x00, 'I', mode, 0x58};
    return _compare(expected, 4);
}

// -----------------------------------------------------------------------------

void test(const char * name, bool (*callback)(void)) {
    _reset();
    Serial.print(name);
    for (uint8_t i=strlen(name); i<TEST_NAME_PADDING; i++) Serial.print(".");
    bool response = (callback)();
    Serial.println(response ? "OK" : "FAIL");

}
void setup() {

    Serial.begin(115200);
    while (!Serial);
    Serial.println();
    Serial.println("Allwize library test suite");
    for (uint8_t i=0; i<TEST_NAME_PADDING+4; i++) Serial.print("-");
    Serial.println();

    mock.callback(_mock_response);
    allwize = new Allwize(mock);

    test("setChannel", test_set_channel);
    test("setChannelPersist", test_set_channel_persist);
    test("setMBusMode", test_set_mbus_mode);
    test("setMBusModePersist", test_set_mbus_mode_persist);

}

void loop() {}
