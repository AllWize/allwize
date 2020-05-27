/*

AllWize - WIZE 2 TheThings.io

Listens to messages on the same channel, data rate and CF and
forwards them to TheThings.io.
This example is meant to run on a Wemos D1 board (ESP8266).

Copyright (C) 2018-2020 by AllWize <github@allwize.io>

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
// Dependencies
// -----------------------------------------------------------------------------

#if not defined(ARDUINO_ARCH_ESP8266)
    #error "This example is meant to run on an ESP8266 board!"
#endif

#include "AllWize.h"
#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>
#include <vector>
#include <ArduinoJson.h>
#include "configuration.h"

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

AsyncMqttClient mqtt;
Ticker mqttTimer;

typedef struct {
    unsigned char id;
    char * token;
    std::vector<const char *> keys;
} node_config_t;
std::vector<node_config_t> _config;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiTimer;

AllWize allwize(RX_PIN, TX_PIN, RESET_PIN);;

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------

void mqttConnect() {
    DEBUG_SERIAL.println("[MQTT] Connecting...");
    mqtt.connect();
}

void mqttOnConnect(bool sessionPresent) {
    DEBUG_SERIAL.println("[MQTT] Connected!");
}

void mqttOnDisconnect(AsyncMqttClientDisconnectReason reason) {
    DEBUG_SERIAL.println("[MQTT] Disconnected!");
    if (WiFi.isConnected()) {
        mqttTimer.detach();
        mqttTimer.once(2, mqttConnect);
    }
}

void mqttSend(const char * topic, const char * payload) {

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "[MQTT] Sending: %s => %s\n", topic, payload);
    DEBUG_SERIAL.print(buffer);

    mqtt.publish(topic, MQTT_QOS, MQTT_RETAIN, payload);

}

void mqttSetup() {
    mqtt.onConnect(mqttOnConnect);
    mqtt.onDisconnect(mqttOnDisconnect);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    #if defined(MQTT_USER) && defined(MQTT_PASS)
        if (strlen(MQTT_USER) > 0 && strlen(MQTT_PASS) > 0) {
            mqtt.setCredentials(MQTT_USER, MQTT_PASS);
        }
    #endif
}

// -----------------------------------------------------------------------------
// TheThings.io
// -----------------------------------------------------------------------------

void ttioSetup() {

    // read configuration
    DynamicJsonBuffer jsonBuffer;
    JsonArray& config = jsonBuffer.parseArray(THETHINGSIO_CONFIG);
    if (!config.success()) {
        DEBUG_SERIAL.println("[TTIO] Error parsing the configuration file!");
        while (true);
    }

    // Parse array of objects
    for (JsonObject& element : config) {

        // Test node integrity
        if (!element.containsKey("id") || !element.containsKey("token") || !element.containsKey("keys")) {
            DEBUG_SERIAL.printf("[TTIO] Missing key for node %d\n", _config.size());
            while (true);
        }

        node_config_t node;
        node.id = element.get<unsigned char>("id");
        node.token = strdup(element.get<const char *>("token"));
        JsonArray& keys = element["keys"];
        for (JsonVariant& key : keys) {
            node.keys.push_back(strdup(key.as<const char *>()));
        }
        _config.push_back(node);
    }

    DEBUG_SERIAL.printf("[TTIO] %d node(s) parsed\n", _config.size());

}

char ttioGet(unsigned char node_id) {
    for (unsigned char i=0; i<_config.size(); i++) {
        if (node_id == _config[i].id) {
            return i;
        }
    }
    return -1;
}

bool ttioSend(allwize_message_t message) {

    if (!mqtt.connected()) return false;

    // We are using the CI field as node_id
    char index = ttioGet(message.ci);
    if (-1 == index) {
        DEBUG_SERIAL.printf("[TTIO] No map for node_id %d\n", message.ci);
        return false;
    }
    node_config_t node = _config[index];
    unsigned char key_count = node.keys.size();

    // Build topic string
    char topic[64];
    snprintf(topic, sizeof(topic), MQTT_TOPIC, node.token);

    // Build payload structure
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonArray& values = root.createNestedArray("values");

    // Parse payload
    char * value;
    unsigned char count = 0;
    value = strtok((char *) message.data, ",");
    while (NULL != value) {

        // check if there is a key for this value
        if (count == key_count) break;

        // Assign the value to the key
        JsonObject& line = values.createNestedObject();
        line["key"] = node.keys[count];
        line["value"] = value;

        // Get new value
        value = strtok (NULL, ",");

        // Update counter
        ++count;

    }

    // Publish message
    String output;
    root.printTo(output);
    mqttSend(topic, output.c_str());

    return true;

}

// -----------------------------------------------------------------------------
// Wize
// -----------------------------------------------------------------------------

void wizeSetup() {

    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize.master();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);

    allwize.dump(DEBUG_SERIAL);

    DEBUG_SERIAL.println("[WIZE] Listening...");

}

void wizeDebugMessage(allwize_message_t message) {

    // Code to pretty-print the message
    char buffer[128];
    snprintf(
        buffer, sizeof(buffer),
        "[WIZE] C: 0x%02X, MAN: %s, ADDR: 0x%02X%02X%02X%02X, TYPE: 0x%02X, VERSION: 0x%02X, CI: 0x%02X, RSSI: 0x%02X, DATA: { ",
        message.c,
        message.man,
        message.address[0], message.address[1],
        message.address[2], message.address[3],
        message.type, message.version,
        message.ci, message.rssi
    );
    DEBUG_SERIAL.print(buffer);

    for (uint8_t i=0; i<message.len; i++) {
        char ch = message.data[i];
        snprintf(buffer, sizeof(buffer), "0x%02X ", ch);
        DEBUG_SERIAL.print(buffer);
    }
    DEBUG_SERIAL.print("}, STR: \"");
    DEBUG_SERIAL.print((char *) message.data);
    DEBUG_SERIAL.println("\"");

}

void wizeLoop() {

    if (allwize.available()) {

        // Get the message
        allwize_message_t message = allwize.read();

        // Show it to console
        wizeDebugMessage(message);

        // Send message to TheThings.io
        ttioSend(message);

    }

}

// -----------------------------------------------------------------------------
// WiFi
// -----------------------------------------------------------------------------

void wifiConnect() {
    DEBUG_SERIAL.println("[WIFI] Connecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void wifiOnConnect(const WiFiEventStationModeGotIP& event) {
    DEBUG_SERIAL.println("[WIFI] Connected!");
    mqttConnect();
}

void wifiOnDisconnect(const WiFiEventStationModeDisconnected& event) {
    DEBUG_SERIAL.println("[WIFI] Disconnected!");
    wifiTimer.detach();
    wifiTimer.once(2, wifiConnect);
}

void wifiSetup() {
    wifiConnectHandler = WiFi.onStationModeGotIP(wifiOnConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(wifiOnDisconnect);
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

void setup() {

    // Setup serial DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    while (!DEBUG_SERIAL && millis() < 5000);
    DEBUG_SERIAL.println();
    DEBUG_SERIAL.println("[MAIN] Wize 2 TheThings.io bridge");
    DEBUG_SERIAL.println();

    mqttSetup();
    ttioSetup();
    wifiSetup();
    wizeSetup();

    wifiConnect();

}

void loop() {

    wizeLoop();

}
