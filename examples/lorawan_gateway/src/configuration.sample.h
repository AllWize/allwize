/*

Configuration file

*/

#pragma once

//------------------------------------------------------------------------------
// General configuration
//------------------------------------------------------------------------------

#define DEBUG_PORT              Serial
#define DEBUG_PORT_BAUDRATE     115200

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
// LoRaWAN server configuration
//------------------------------------------------------------------------------

#define LORAWAN_SERVER          IPAddress(52,169,76,203)
#define LORAWAN_PORT            1700
#define LORAWAN_LATITUDE        0
#define LORAWAN_LONGITUDE       0
#define LORAWAN_ALTITUDE        0
#define LORAWAN_GATEWAY_TYPE    "..."
#define LORAWAN_EMAIL           "..."
#define LORAWAN_DESCRIPTION     "..."

//------------------------------------------------------------------------------
// NTP configuration
//------------------------------------------------------------------------------

#define NTP_SERVER              "pool.ntp.org"
#define NTP_TIMEZONE_OFFSET     0
#define NTP_DAYLIGHT_OFFSET     0
#define NTP_SYNC_INTERVAL       10
#define NTP_UPDATE_INTERVAL     1800
