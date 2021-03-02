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

Copyright (C) 2018-2021 by AllWize <github@allwize.io>

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
#include "SoftwareSerial.h"
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

AllWize allwize(RX_PIN, TX_PIN, RESET_PIN);

// -----------------------------------------------------------------------------
// Utils
// -----------------------------------------------------------------------------

String bin2hex(uint8_t * bin, uint8_t len) {
    char b[3];
    String output = String("");
    for (uint8_t i = 0; i < len; i++) {
        sprintf(b, "%02X", bin[i]);
        output += String(b);
    }
    return output;
}

// -----------------------------------------------------------------------------
// MQTT
// -----------------------------------------------------------------------------

void mqttConnect() {
    DEBUG_SERIAL.printf("[MQTT] Connecting to %s:%d...\n", MQTT_HOST, MQTT_PORT);
    mqtt.connect();
}

void mqttOnConnect(bool sessionPresent) {
    DEBUG_SERIAL.printf("[MQTT] Connected!\n");
    ping();
}

void mqttOnDisconnect(AsyncMqttClientDisconnectReason reason) {
    DEBUG_SERIAL.printf("[MQTT] Disconnected!\n");
    if (WiFi.isConnected()) {
        mqttTimer.detach();
        mqttTimer.once(2, mqttConnect);
    }
}

void mqttSend(const char * topic, const char * payload) {
    DEBUG_SERIAL.printf("[MQTT] Sending: %s => %s\n", topic, payload);
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

    // Init AllWize object
    allwize.begin();
    if (!allwize.waitForReady()) {
        DEBUG_SERIAL.printf("[WIZE] Error connecting to the module, check your wiring!\n");
        while (true) delay(1);
    }

    allwize.master();
    allwize.setChannel(WIZE_CHANNEL, true);
    allwize.setPower(WIZE_POWER);
    allwize.setDataRate(WIZE_DATARATE);

    DEBUG_SERIAL.printf("[WIZE] Module type: %s\n", allwize.getModuleTypeName().c_str());
    DEBUG_SERIAL.printf("[WIZE] MBUS mode: 0x%2X\n", allwize.getMode());
    DEBUG_SERIAL.printf("[WIZE] Channel: %d\n", allwize.getChannel());
    DEBUG_SERIAL.printf("[WIZE] Datarate: %d (%d bps)\n", allwize.getDataRate(), allwize.getDataRateSpeed(allwize.getDataRate()));
    DEBUG_SERIAL.printf("[WIZE] Listening...\n");

}

void wizeDebugMessage(allwize_message_t message) {

    // Code to pretty-print the message
    DEBUG_SERIAL.printf(
        "[WIZE] ADDR: 0x%s, RSSI: %d, DATA: 0x%s\n",
        bin2hex(message.address, 4).c_str(),
        (int16_t) message.rssi / -2,
        bin2hex(message.data, message.len).c_str()
    );

}

void ping() {

    DynamicJsonDocument root(512);
    
    JsonObject gateway = root.createNestedObject("gateway");
    gateway["mid"] = allwize.getMID();
    gateway["uid"] = allwize.getUID();
    //gateway["sn"] = allwize.getSerialNumber();

    char topic[32];
    snprintf(topic, sizeof(topic), "gateway/%s%s/ping", allwize.getMID().c_str(), allwize.getUID().c_str());
    String payload;
    serializeJson(root, payload);
    mqttSend(topic, payload.c_str());

}

void wizeMQTTParse(allwize_message_t message) {

    DynamicJsonDocument root(512);
    
    root["app"] = message.wize_application;
    root["net"] = message.wize_network_id;
    root["uid"] = bin2hex(message.address, 4);
    root["cpt"] = message.wize_counter;

    JsonObject metadata = root.createNestedObject("metadata");
    metadata["ch"] = allwize.getChannel();
    metadata["freq"] = allwize.getFrequency(allwize.getChannel());
    metadata["dr"] = allwize.getDataRateSpeed(allwize.getDataRate());
    metadata["toa"] = 8000.0 * message.len / allwize.getDataRateSpeed(allwize.getDataRate());
    
    JsonObject gateway = root.createNestedObject("gateway");
    gateway["mid"] = allwize.getMID();
    gateway["uid"] = allwize.getUID();
    //gateway["sn"] = allwize.getSerialNumber();
    gateway["rssi"] = message.rssi / -2;

    root["payload"] = bin2hex(message.data, message.len);

    char topic[32];

    #if DECODE_PAYLOAD

        JsonObject fields = root.createNestedObject("fields");

        #if PAYLOAD_ENCODING == PAYLOAD_CSV

            // Parse a comma-separated payload
            uint8_t field = 1;
            char * payload;
            payload = strtok((char *) message.data, ",");

            // While there is a field
            while (NULL != payload) {

                snprintf(topic, sizeof(topic), "field_%d", field);
                fields[topic] = String(payload);

                // Get new token and update field counter
                payload = strtok (NULL, ",");
                field++;

            }

        #endif // PAYLOAD_CSV

        #if PAYLOAD_ENCODING == PAYLOAD_MBUS

            // Parse payload
            MBUSPayload payload;
            DynamicJsonDocument jsonBuffer(512);
            JsonArray input = jsonBuffer.createNestedArray();
            payload.decode(message.data, message.len, input);

            char format[10];
            char value[16];

            // Walk JsonArray
            for (JsonObject element: input) {

                uint32_t vif = element["vif"].as<uint32_t>();
                int8_t scalar = element["scalar"].as<int>();
                float value_scaled = element["value_scaled"].as<float>();

                snprintf(topic, sizeof(topic), "%06X", vif);
                
                uint8_t decimals = (scalar > 0) ? 0 : -scalar;
                if (decimals == 0) {
                    snprintf(value, sizeof(value), "%u", (int) value_scaled);
                } else {
                    snprintf(format, sizeof(format), "%%.%df", decimals);
                    snprintf(value, sizeof(value), format, value_scaled);
                }
                
                fields[topic] = String(value);

            }

        #endif // PAYLOAD_MBUS

        #if PAYLOAD_ENCODING == PAYLOAD_LPP

            // Parse payload
            CayenneLPP payload(1);
            DynamicJsonDocument jsonBuffer(512);
            JsonArray input = jsonBuffer.createNestedArray();
            payload.decode(message.data, message.len, input);

            // Walk JsonArray
            for (JsonObject element: input) {
                JsonVariant v = element["value"];
                if (v.is<JsonObject>()) {
                    for (JsonPair kv : v.as<JsonObject>()) {
                        fields[kv.key()] = kv.value().as<float>();
                    }
                } else {
                    fields[element["name"].as<char *>()] = element["value"].as<float>();

                }
            }

        #endif // PAYLOAD_LPP

    #endif // DECODE_PAYLOAD

    snprintf(topic, sizeof(topic), "gateway/%s%s/uplink", allwize.getMID().c_str(), allwize.getUID().c_str());
    String output;
    serializeJson(root, output);
    mqttSend(topic, output.c_str());

}

void wizeLoop() {

    if (allwize.available()) {

        // Get the message
        allwize_message_t message = allwize.read();

        // Show it to console
        wizeDebugMessage(message);

        // Parse and send via MQTT
	    if (mqtt.connected()) {
            wizeMQTTParse(message);
        }

    }

}

// -----------------------------------------------------------------------------
// WiFi
// -----------------------------------------------------------------------------

void wifiConnect() {
    DEBUG_SERIAL.printf("[WIFI] Connecting to %s...\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void wifiOnConnect(const WiFiEventStationModeGotIP& event) {
    DEBUG_SERIAL.printf("[WIFI] Connected!\n");
    mqttConnect();
}

void wifiOnDisconnect(const WiFiEventStationModeDisconnected& event) {
    DEBUG_SERIAL.printf("[WIFI] Disconnected!\n");
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
    DEBUG_SERIAL.printf("\n[MAIN] Wize 2 MQTT bridge\n\n");

    mqttSetup();
    wifiSetup();
    wizeSetup();

    wifiConnect();

}

void loop() {

    // Listen to messages
    wizeLoop();

    // PING
    #if PING_INTERVAL
        static unsigned long last_ping = 0;
        if (millis() - last_ping > PING_INTERVAL) {
            last_ping = millis();
            ping();
        }
    #endif

    delay(1);

}
