/*

FORWARDER MODULE

*/

#include "forwarder.h"
#include "wifi.h"
#include "ntp.h"
#include "debug.h"
#include "wize.h"
#include "configuration.h"

#include "AllWize.h"
#include <base64.h>
#include <Ticker.h>
#include <WiFiUdp.h>

Ticker _forwarder_ticker;
WiFiUDP _forwarder_udp;
uint8_t eui[8] = {0xFF};

struct {
    uint16_t rxnb = 0; // Number of radio packets received (unsigned integer)
    uint16_t rxok = 0; // Number of radio packets received with a valid PHY CRC
    uint16_t rxfw = 0; // Number of radio packets forwarded (unsigned integer)
    uint16_t acks = 0; // Number of acknowledges received
    uint16_t dwnb = 0; // Number of downlink datagrams received (unsigned integer)
    uint16_t txnb = 0; // Number of packets emitted (unsigned integer)
} _forwarder_stats;

void forwarderSend(char * data) {

    // Binary header
    uint8_t header[12];
    header[0] = 1;              // protocol version
    header[1] = random(256);    // random for ack
    header[2] = random(256);    // random for ack
    header[3] = 0;              // transaction type
    memcpy(&header[4], eui, 8); // gateway eui

    DEBUG_MSG("[FORWARDER] Sending: ");
    for (uint8_t i=0; i<12; i++) {
        DEBUG_MSG("%02x", header[i]);
    }
    DEBUG_MSG(" %s\n", data);

    _forwarder_udp.beginPacket(FORWARDER_SERVER, FORWARDER_PORT);
    _forwarder_udp.write(header, 12);
    _forwarder_udp.write((uint8_t *) data, strlen(data));
    _forwarder_udp.endPacket();

}

void forwarderPing() {

    // Check connection
    if (!wifiConnected()) return;
    if (!ntpSynced()) return;

    // Get % of ACK received
    float ackr = (0 == _forwarder_stats.rxfw) ? 0 : 100.0 * (float)_forwarder_stats.acks / (float)_forwarder_stats.rxfw;

    // Build JSON payload
    String timestamp = ntpDateTime().c_str();

    // Build JSON payload
    char buffer[512];
    snprintf_P(
        buffer, sizeof(buffer), 
        PSTR("{\"stat\":{\"time\":\"%s GMT\",\"lati\":%.5f,\"long\":%.5f,\"alti\":%d,\"rxnb\":%d,\"rxok\":%d,\"rxfw\":%d,\"ackr\":%.2f,\"dwnb\":%d,\"txnb\":%d,\"pfrm\":\"%s\",\"mail\":\"%s\",\"desc\":\"%s\"}}"),
        timestamp.c_str(), FORWARDER_LATITUDE, FORWARDER_LONGITUDE, FORWARDER_ALTITUDE,
        _forwarder_stats.rxnb,_forwarder_stats.rxok, _forwarder_stats.rxfw, ackr, _forwarder_stats.dwnb, _forwarder_stats.txnb,
        FORWARDER_GATEWAY_TYPE, FORWARDER_EMAIL, FORWARDER_DESCRIPTION
    );

    // Send frame
    forwarderSend(buffer);

}

void forwarderMessage(allwize_message_t message) {

    // Update RX counters
    _forwarder_stats.rxnb = _forwarder_stats.rxnb + 1;
    _forwarder_stats.rxok = _forwarder_stats.rxok + 1;

    // Check connection & time
    if (!wifiConnected()) return;
    if (!ntpSynced()) return;

    // Get BASE64 of payload
    String data = base64::encode(message.data, message.len, false);

    // Get current time
    String timestamp = ntpDateTime().c_str();
    timestamp.replace(" ", "T");

    // Build JSON payload
    char buffer[512];
    snprintf_P(
        buffer, sizeof(buffer), 
        PSTR("{\"rxpk\":[{\"tmst\":%lu,\"time\":\"%s.00000Z\",\"chan\":%d,\"rfch\":%d,\"freq\":%.5f,\"stat\":%d,\"modu\":\"FSK\",\"datr\":%d,\"codr\":\"WIZE/DR%d\",\"rssi\":%d,\"lsnr\":0.0,\"size\":%d,\"data\":\"%s\"}]}"),
        now(), timestamp.c_str(), 
        WIZE_CHANNEL, 0, wizeFrequency(WIZE_CHANNEL), 1,
        wizeDataRateSpeed(WIZE_DATARATE), WIZE_DATARATE,
        (int16_t) message.rssi / -2, message.len, data.c_str()
    );

    // Send frame
    forwarderSend(buffer);

    // Update FW counter
    _forwarder_stats.rxfw =_forwarder_stats.rxfw + 1;

}

void forwarderSetup() {
    
    // Set the ping message every 30 seconds
    _forwarder_ticker.attach(30, forwarderPing);

    // Build the device EUI
    uint8_t mac[WL_MAC_ADDR_LENGTH] = {0};
    WiFi.macAddress(mac);
    eui[0] = mac[0];
    eui[1] = mac[1];
    eui[2] = mac[2];
    eui[3] = 0xFF;
    eui[4] = 0xFF;
    eui[5] = mac[3];
    eui[6] = mac[4];
    eui[7] = mac[5];
    DEBUG_MSG(
        "[FORWARDER] Gateway eui-%02x%02x%02x%02x%02x%02x%02x%02x\n",
        eui[0], eui[1], eui[2], eui[3],
        eui[4], eui[5], eui[6], eui[7]
    );

    // Init random seed
    randomSeed(analogRead(0));

}
