/*

Allwize - WIZE 2 MQTT Bridge

Listens to messages on the same channel, data rate and CF and
forwards them to an MQTT broker.
This example is meant to run on a Wemos D1 board (ESP8266).

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

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

#if not defined(ARDUINO_ARCH_ESP8266)
    #error "This example is meant to run on an ESP8266 board!"
#endif

#define RESET_PIN   14
#define RX_PIN      12
#define TX_PIN      13
#include            <SoftwareSerial.h>
SoftwareSerial      module(RX_PIN, TX_PIN);
#define debug       Serial

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps
#define WIZE_NETWORK_ID         0x46

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

#include "credentials.h"
#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>
#include "Allwize.h"

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiTimer;

AsyncMqttClient mqtt;
Ticker mqttTimer;

Allwize * allwize;

#define USE_SNIFFER 0
#if USE_SNIFFER
    #include "SerialSniffer.h"
    SerialSniffer * sniffer;
#endif

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------

void mqttConnect() {
    debug.println("[MQTT] Connecting...");
    mqtt.connect();
}

void mqttOnConnect(bool sessionPresent) {
    debug.println("[MQTT] Connected!");
}

void mqttOnDisonnect(AsyncMqttClientDisconnectReason reason) {
    debug.println("[MQTT] Disconnected!");
    if (WiFi.isConnected()) {
        mqttTimer.detach();
        mqttTimer.once(2, mqttConnect);
    }
}

void mqttSetup() {
    mqtt.onConnect(mqttOnConnect);
    mqtt.onDisconnect(mqttOnDisonnect);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    #if defined(MQTT_USER) & defined(MQTT_PASS)
        if (strlen(MQTT_USER) > 0 && strlen(MQTT_PASS) > 0) {
            mqtt.setCredentials(MQTT_USER, MQTT_PASS);
        }
    #endif
}

// -----------------------------------------------------------------------------
// WiFi
// -----------------------------------------------------------------------------

void wifiConnect() {
    debug.println("[WIFI] Connecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void wifiOnConnect(const WiFiEventStationModeGotIP& event) {
    debug.println("[WIFI] Connected!");
    mqttConnect();
}

void wifiOnDisconnect(const WiFiEventStationModeDisconnected& event) {
    debug.println("[WIFI] Disconnected!");
    wifiTimer.detach();
    wifiTimer.once(2, wifiConnect);
}

void wifiSetup() {
    wifiConnectHandler = WiFi.onStationModeGotIP(wifiOnConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(wifiOnDisconnect);
}

// -----------------------------------------------------------------------------
// Wize
// -----------------------------------------------------------------------------

void wizeSetup() {

    #if USE_SNIFFER
        sniffer = new SerialSniffer(module, debug);
        allwize = new Allwize(*sniffer, RESET_PIN);
    #else
        allwize = new Allwize(module, RESET_PIN);
    #endif

    allwize->reset();
    module.begin(19200);
    while (!allwize->ready());
    allwize->begin();

    allwize->master();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);
    allwize->setControlField(WIZE_NETWORK_ID);

    allwize->dump(debug);

    debug.println("[WIZE] Listening...");

}

void wizeLoop() {

    if (allwize->available()) {

        allwize_message_t message = allwize->read();

        // Code to pretty-print the message
        char buffer[64];
        char ascii[message.len+1];

        snprintf(buffer, sizeof(buffer), "[WIZE] C: 0x%02X, CI: 0x%02X, RSSI: 0x%02X, DATA: { ", message.c, message.ci, message.rssi);
        debug.print(buffer);

        for (uint8_t i=0; i<message.len; i++) {
            char ch = message.data[i];
            snprintf(buffer, sizeof(buffer), "0x%02X ", ch);
            debug.print(buffer);
            ascii[i] = (31 < ch && ch < 127) ? ch : ' ';
        }
        ascii[message.len] = 0;
        debug.print("}, STR: \"");
        debug.print(ascii);
        debug.println("\"");

        // Sending message via MQTT
        if (mqtt.connected()) {

            // Init field counter
            uint8_t field = 1;

            // Parse a comma-separated payload
            char * payload;
            payload = strtok((char *) message.data, ",");

            // While there is a field
            while (NULL != payload) {

                // Build topic string with CI and field number
                char topic[32];
                snprintf(topic, sizeof(topic), MQTT_TOPIC, message.ci, field);

                // Publish message
                snprintf(buffer, sizeof(buffer), "[WIZE] MQTT message: %s => %s\n", topic, payload);
                debug.print(buffer);
                mqtt.publish(topic, MQTT_QOS, MQTT_RETAIN, payload);

                // Get new token and update field counter
                payload = strtok (NULL, ",");
                field++;

            }

        }

    }

}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Setup serial debug
    debug.begin(115200);
    while (!debug && millis() < 5000);
    debug.println();
    debug.println("[MAIN] Wize 2 MQTT bridge");
    debug.println();

    mqttSetup();
    wifiSetup();
    wizeSetup();

    wifiConnect();

}

void loop() {

    wizeLoop();

}
