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

#define RESET_PIN               14
#define RX_PIN                  12
#define TX_PIN                  13
#define DEBUG_SERIAL            Serial

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps

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

void mqttOnDisonnect(AsyncMqttClientDisconnectReason reason) {
    DEBUG_SERIAL.println("[MQTT] Disconnected!");
    if (WiFi.isConnected()) {
        mqttTimer.detach();
        mqttTimer.once(2, mqttConnect);
    }
}

void mqttSetup() {
    mqtt.onConnect(mqttOnConnect);
    mqtt.onDisconnect(mqttOnDisonnect);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    #if defined(MQTT_USER) && defined(MQTT_PASS)
        if (strlen(MQTT_USER) > 0 && strlen(MQTT_PASS) > 0) {
            mqtt.setCredentials(MQTT_USER, MQTT_PASS);
        }
    #endif
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
// Wize
// -----------------------------------------------------------------------------

void wizeSetup() {

    allwize = new Allwize(RX_PIN, TX_PIN, RESET_PIN);
    allwize->begin();
    if (!allwize->waitForReady()) {
        DEBUG_SERIAL.println("Error connecting to the module, check your wiring!");
        while (true);
    }

    allwize->master();
    allwize->setChannel(WIZE_CHANNEL, true);
    allwize->setPower(WIZE_POWER);
    allwize->setDataRate(WIZE_DATARATE);

    allwize->dump(DEBUG_SERIAL);

    DEBUG_SERIAL.println("[WIZE] Listening...");

}

void wizeLoop() {

    if (allwize->available()) {

        allwize_message_t message = allwize->read();

        // Code to pretty-print the message
        char buffer[64];
        char ascii[message.len+1];

        snprintf(buffer, sizeof(buffer), "[WIZE] C: 0x%02X, CI: 0x%02X, RSSI: 0x%02X, DATA: { ", message.c, message.ci, message.rssi);
        DEBUG_SERIAL.print(buffer);

        for (uint8_t i=0; i<message.len; i++) {
            char ch = message.data[i];
            snprintf(buffer, sizeof(buffer), "0x%02X ", ch);
            DEBUG_SERIAL.print(buffer);
            ascii[i] = (31 < ch && ch < 127) ? ch : ' ';
        }
        ascii[message.len] = 0;
        DEBUG_SERIAL.print("}, STR: \"");
        DEBUG_SERIAL.print(ascii);
        DEBUG_SERIAL.println("\"");

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
                DEBUG_SERIAL.print(buffer);
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
