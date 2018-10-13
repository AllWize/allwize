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
#define RX_PIN                  12
#define TX_PIN                  13

//------------------------------------------------------------------------------
// Wize configuration
//------------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04
#define WIZE_POWER              POWER_20dBm
#define WIZE_DATARATE           DATARATE_2400bps

//------------------------------------------------------------------------------
// MQTT configuration
//------------------------------------------------------------------------------

#define MQTT_HOST               "192.168.2.2"
#define MQTT_PORT               1883
#define MQTT_USER               ""
#define MQTT_PASS               ""
#define MQTT_QOS                2
#define MQTT_RETAIN             0
#define MQTT_TOPIC              "team/%u/field_%u"    // CI, field#
