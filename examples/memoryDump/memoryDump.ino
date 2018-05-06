#include "Allwize.h"

class AllwizeWrap : public Allwize {
    public:
        AllwizeWrap(Stream& stream, uint8_t reset_gpio = 0xFF): Allwize(stream, reset_gpio) {};
        uint8_t getMemory(uint8_t address) { return _getMemory(address); }

};

AllwizeWrap * allwize;

#ifdef ARDUINO_AVR_UNO
    #define RX_PIN      8
    #define TX_PIN      9
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
#endif

#ifdef ARDUINO_AVR_LEONARDO
    #define module      Serial1
#endif

#ifdef ARDUINO_ARCH_ESP8266
    #define RX_PIN      12
    #define TX_PIN      13
    #include <SoftwareSerial.h>
    SoftwareSerial module(RX_PIN, TX_PIN);
#endif

void setup() {

    Serial.begin(115200);
    while (!Serial);

    Serial.println();
    Serial.println("Allwize - Module Memory Dump");
    Serial.println();

    module.begin(19200);
    allwize = new AllwizeWrap(module);

    char buffer[10];

    Serial.print("      ");
    for (uint16_t address = 0; address <= 0x0F; address++) {
        snprintf(buffer, sizeof(buffer), " %02X", address);
        Serial.print(buffer);
    }
    Serial.println();
    Serial.print("------------------------------------------------------");

    for (uint16_t address = 0; address <= 0xFF; address++) {
        if ((address % 16) == 0) {
            snprintf(buffer, sizeof(buffer), "\n0x%02X: ", address);
            Serial.print(buffer);
        }
        snprintf(buffer, sizeof(buffer), " %02X", allwize->getMemory(address));
        Serial.print(buffer);
    }

    Serial.println();
    Serial.println();
    Serial.println("Done");

}

void loop() {}
