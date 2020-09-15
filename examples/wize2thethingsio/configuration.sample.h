/*

Configuration file

*/

#pragma once

//------------------------------------------------------------------------------
// General configuration
//------------------------------------------------------------------------------

#define DEBUG_SERIAL            Serial

//------------------------------------------------------------------------------
// WiFi credentials
//------------------------------------------------------------------------------

#define WIFI_SSID               "..."
#define WIFI_PASSWORD           "..."

//------------------------------------------------------------------------------
// Radio module connections
//------------------------------------------------------------------------------

#define RESET_PIN               14
#define RX_PIN                  5
#define TX_PIN                  4

//------------------------------------------------------------------------------
// Wize configuration
//------------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps

//------------------------------------------------------------------------------
// TheThings.IO configuration
//------------------------------------------------------------------------------

#define MQTT_HOST               "mqtt.thethings.io"
#define MQTT_PORT               1883
#define MQTT_QOS                2
#define MQTT_RETAIN             0
#define MQTT_TOPIC              "v2/things/%s"    // placeholder for the TOKEN

const char THETHINGSIO_CONFIG[] = R"raw([
    {"id": "1", "token": "...", "keys": ["co", "no2"] },
    {"id": "8", "token": "...", "keys": ["temperature", "humidity"] }
])raw";
