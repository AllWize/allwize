#include "Allwize.h"
#include "StreamInjector.h"

StreamInjector mock;
Allwize * allwize;

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

    static uint8_t pending_payload = 1;

    if (pending_payload > 0) --pending_payload;
    if (0 == pending_payload) mock.inject('>');

    _rx_buffer[_rx_count++] = ch;

}

// -----------------------------------------------------------------------------

bool test_set_channel(void) {
    allwize->setChannel(3);
    uint8_t expected[4] = {0x00, 'C', 3, 0xFF};
    return _compare(expected, 4);
}

// -----------------------------------------------------------------------------

void test(const char * name, bool (*callback)(void)) {
    _reset();
    Serial.print(name);
    Serial.print(": ");
    bool response = (callback)();
    Serial.println(response ? "OK" : "FAIL");

}
void setup() {

    Serial.begin(115200);
    while (!Serial);
    Serial.println();
    Serial.println("Allwize library test suite");
    Serial.println("--------------------------");
    Serial.println();

    mock.callback(_mock_response);
    allwize = new Allwize(mock);

    test("setChannel", test_set_channel);

}

void loop() {}
