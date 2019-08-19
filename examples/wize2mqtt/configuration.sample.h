/*

Configuration file

*/

#pragma once

//------------------------------------------------------------------------------
// Payload data format
// You have to uncomment just one of these defines
//------------------------------------------------------------------------------

#define PAYLOAD_CSV
//#define PAYLOAD_MBUS
//#define PAYLOAD_LPP

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
// MQTT configuration
//------------------------------------------------------------------------------

#define MQTT_HOST               "192.168.2.2"
#define MQTT_PORT               1883
#define MQTT_USER               ""
#define MQTT_PASS               ""
#define MQTT_QOS                2
#define MQTT_RETAIN             0
