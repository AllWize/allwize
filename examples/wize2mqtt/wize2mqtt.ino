/*

AllWize - WIZE 2 MQTT Bridge
Supports CSV, MBUSPayload and CayenneLPP payload frames

Listens to messages on the same channel, data rate and CF and
forwards them to an MQTT broker.

It supports different application frame formats:
* CSV: comma separated list of values, each field will be named as field_#
* MBUSPayload: https://github.com/AllWize/mbus-payload
* CayenneLPP: https://github.com/ElectronicCats/CayenneLPP

Sends one MQTT message per field.
This example is meant to run on a Wemos D1 board (ESP8266).

Copyright (C) 2018-2019 by AllWize <github@allwize.io>

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

#include "SoftwareSerial.h"
#include "AllWize.h"
#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>
#include "configuration.h"

#ifdef PAYLOAD_MBUS
#include "MBUSPayload.h"
#include "ArduinoJson.h"
#endif

#ifdef PAYLOAD_LPP
#include "CayenneLPP.h"
#include "ArduinoJson.h"
#endif

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

AsyncMqttClient mqtt;
Ticker mqttTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiTimer;

AllWize * allwize;

// -----------------------------------------------------------------------------
// Formatting
// -----------------------------------------------------------------------------

char * snfloat(char * buffer, uint8_t len, uint8_t decimals, float value) {

    bool negative = value < 0;
    if (negative) value = -value;

    uint32_t mul = 1;
    for (uint8_t i=0; i<decimals; i++) mul *= 10;

    uint32_t value_int = int(value);
    uint32_t value_dec = int(mul * (value - value_int));

    char format[20];
    snprintf(format, sizeof(format), "%s%%lu.%%0%ulu", negative ? "-" : "", decimals);
    snprintf(buffer, len, format, value_int, value_dec);

    return buffer;

}

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
// Wize
// -----------------------------------------------------------------------------

void wizeSetup() {

    allwize = new AllWize(RX_PIN, TX_PIN, RESET_PIN);
    allwize->begin();
    if (!allwize->waitForReady()) {
        DEBUG_SERIAL.println("[WIZE] Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize->master();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);

    allwize->dump(DEBUG_SERIAL);

    DEBUG_SERIAL.println("[WIZE] Listening...");

}

void wizeDebugMessage(allwize_message_t message) {

    // Code to pretty-print the message
    char buffer[128];
    snprintf(
        buffer, sizeof(buffer),
        "[WIZE] ADDR: 0x%02X%02X%02X%02X, RSSI: %d, DATA: ",
        message.address[0], message.address[1],
        message.address[2], message.address[3],
        (int16_t) message.rssi / -2
    );
    DEBUG_SERIAL.print(buffer);

    for (uint8_t i=0; i<message.len; i++) {
        char ch = message.data[i];
        snprintf(buffer, sizeof(buffer), "%02X", ch);
        DEBUG_SERIAL.print(buffer);
    }
    DEBUG_SERIAL.println();

}

void wizeMQTTParse(allwize_message_t message) {

    char uid[10];
    snprintf(
        uid, sizeof(uid), "%02X%02X%02X%02X",
        message.address[0], message.address[1],
        message.address[2], message.address[3]
    );

    // RSSI
    char topic[32];
    snprintf(topic, sizeof(topic), "/device/%s/rssi", uid);
    mqttSend(topic, String(message.rssi / -2).c_str());

    #ifdef PAYLOAD_CSV

        // Parse a comma-separated payload
        uint8_t field = 1;
        char * payload;
        payload = strtok((char *) message.data, ",");

        // While there is a field
        while (NULL != payload) {

            // Publish message
            snprintf(topic, sizeof(topic), "/device/%s/field_%d", uid, field);
            mqttSend(topic, payload);

            // Get new token and update field counter
            payload = strtok (NULL, ",");
            field++;

        }

    #endif // PAYLOAD_CSV

    #ifdef PAYLOAD_MBUS

        // Parse payload
        MBUSPayload payload;
        DynamicJsonDocument jsonBuffer(512);
        JsonArray root = jsonBuffer.createNestedArray();
        payload.decode(message.data, message.len, root);

        // Walk JsonArray
        for (uint8_t i = 0; i<root.size(); i++) {
            
            snprintf(topic, sizeof(topic), "/device/%s/%06X", uid, (uint32_t) root[i]["vif"]);
            
            char value[12];
            uint8_t decimals = ((int32_t) root[i]["scalar"] > 0) ? 0 : - ((int32_t) root[i]["scalar"]);
            if (decimals == 0) {
                snprintf(value, sizeof(value), "%u", (uint32_t) root[i]["value_scaled"]);
                mqttSend(topic, value);
            } else {
                dtostrf(root[i]["value_scaled"], sizeof(value)-1, decimals, value);
                uint8_t start = 0;
                while (value[start] == ' ') start++;
                mqttSend(topic, &value[start]);
            }


        }

    #endif // PAYLOAD_MBUS

    #ifdef PAYLOAD_LPP

        // Parse payload
        CayenneLPP payload(1);
        DynamicJsonDocument jsonBuffer(512);
        JsonArray root = jsonBuffer.createNestedArray();
        payload.decode(message.data, message.len, root);

        char value[16];
        
        // Walk JsonArray
        for (JsonObject element: root) {
            JsonVariant v = element["value"];
            if (v.is<JsonObject>()) {
                for (JsonPair kv : v.as<JsonObject>()) {

                    const char * name = kv.key().c_str();
                    dtostrf(kv.value().as<float>(), sizeof(value)-1, 4, value);
                    uint8_t start = 0; while (value[start] == ' ') start++;
                    snprintf(topic, sizeof(topic), "/device/%s/%s", uid, name);
                    mqttSend(topic, &value[start]);

                }
            } else {

                const char * name = element["name"].as<char*>();
                dtostrf(element["value"].as<float>(), sizeof(value)-1, 2, value);
                uint8_t start = 0; while (value[start] == ' ') start++;
                snprintf(topic, sizeof(topic), "/device/%s/%s", uid, name);
                mqttSend(topic, &value[start]);

            }
        }

    #endif // PAYLOAD_LPP

}

void wizeLoop() {

    if (allwize->available()) {

        // Get the message
        allwize_message_t message = allwize->read();

        // Show it to console
        wizeDebugMessage(message);

        // Parse and send via MQTT
	    if (mqtt.connected()) wizeMQTTParse(message);

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
    DEBUG_SERIAL.println("[MAIN] Wize 2 MQTT bridge");
    DEBUG_SERIAL.println();

    mqttSetup();
    wifiSetup();
    wizeSetup();

    wifiConnect();

}

void loop() {

    wizeLoop();

}
