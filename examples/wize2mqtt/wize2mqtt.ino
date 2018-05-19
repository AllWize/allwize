/*

Allwize - WIZE 2 MQTT Bridge

This example is meant to run on a Wemos D1 board (ESP8266) and it
forwards any message received to an MQTT topic

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
// Board definitions
// -----------------------------------------------------------------------------

#if not defined(ARDUINO_ARCH_ESP8266)
    #error "This example is meant to run on an ESP8266 board!"
#endif

#define RX_PIN      12
#define TX_PIN      13
#include            <SoftwareSerial.h>
SoftwareSerial      module(RX_PIN, TX_PIN);
#define debug       Serial

// -----------------------------------------------------------------------------
// Config & globals
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
    if (strlen(MQTT_USER) > 0 && strlen(MQTT_PASS) > 0) {
        mqtt.setCredentials(MQTT_USER, MQTT_PASS);
    }
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

    module.begin(19200);
    #if USE_SNIFFER
        sniffer = new SerialSniffer(module, debug);
        allwize = new Allwize(*sniffer);
    #else
        allwize = new Allwize(module);
    #endif
    allwize->begin();
    while (!allwize->ready());

    allwize->master();
    allwize->setChannel(4, true);
    allwize->setPower(5);
    allwize->setDataRate(1);
    allwize->setControlField(0x46);

    allwize->dump(debug);

    debug.println("[WIZE] Listening...");

}

void wizeLoop() {

    if (allwize->available()) {

        allwize_message_t message = allwize->read();

        // Code to pretty-print the message
        char buffer[64];
        char ascii[message.len+1];

        snprintf(buffer, sizeof(buffer), "[WIZE] C: %02X, CI: %02X, RSSI: %02X, DATA: { ", message.c, message.ci, message.rssi);
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
            char payload[32];
            snprintf(payload, sizeof(payload), MQTT_PAYLOAD, (char *) message.data);
            mqtt.publish(MQTT_TOPIC, MQTT_QOS, MQTT_RETAIN, payload);
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
