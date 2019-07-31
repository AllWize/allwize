/*

WIZE MODULE

*/

#include "wize.h"
#include "debug.h"
#include "forwarder.h"
#include "configuration.h"

#include "AllWize.h"

AllWize * allwize;

double wizeFrequency(uint8_t channel) {
    return allwize->getFrequency(channel);
}

uint16_t wizeDataRateSpeed(uint8_t dr) {
    return allwize->getDataRateSpeed(dr);
}

void wizeSetup() {

    // Create and init AllWize object
    allwize = new AllWize(RX_PIN, TX_PIN, RESET_PIN);
    allwize->begin();
    if (!allwize->waitForReady()) {
        DEBUG_MSG("[WIZE] Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize->master();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setDataRate(WIZE_DATARATE);

    #if defined(DEBUG_PORT)
    //allwize->dump(DEBUG_PORT);
    #endif

    DEBUG_MSG("[WIZE] Listening... CH %d, DR %d\n", allwize->getChannel(), allwize->getDataRate());

}

void wizeDebugMessage(allwize_message_t message) {

    // Code to pretty-print the message
    DEBUG_MSG(
        "[WIZE] C: 0x%02X, MAN: %s, ADDR: 0x%02X%02X%02X%02X, TYPE: 0x%02X, VERSION: 0x%02X, CI: 0x%02X, RSSI: %3d, DATA: ",
        message.c,
        message.man,
        message.address[0], message.address[1],
        message.address[2], message.address[3],
        message.type, message.version,
        message.ci, (int16_t) message.rssi / -2
    );

    for (uint8_t i=0; i<message.len; i++) {
        DEBUG_MSG("%02x", message.data[i]);
    }

    DEBUG_MSG("\n");

}

void wizeLoop() {

    if (allwize->available()) {

        // Get the message
        allwize_message_t message = allwize->read();

        // Show it to console
        wizeDebugMessage(message);

        // Forward it
        forwarderMessage(message);

    }

}
