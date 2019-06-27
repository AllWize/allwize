/*

Configuration file

*/

#pragma once

//------------------------------------------------------------------------------
// General configuration
//------------------------------------------------------------------------------

#define DEBUG_PORT              Serial                      // Serial debug port
#define DEBUG_PORT_BAUDRATE     115200                      // Serial debuf baud rate

//------------------------------------------------------------------------------
// WiFi credentials
//------------------------------------------------------------------------------

#define WIFI_SSID               "..."                       // SSID to connect to
#define WIFI_PASSWORD           "..."                       // Wifi password

//------------------------------------------------------------------------------
// Radio module connections
//------------------------------------------------------------------------------

#define RESET_PIN               14                          // GPIO for the reset pad in the RC1701HP module
#define RX_PIN                  5                           // GPIO connected to the TX pad in the RC1701HP module
#define TX_PIN                  4                           // GPIO connected to the RX pad in the RC1701HP module

//------------------------------------------------------------------------------
// Wize configuration
//------------------------------------------------------------------------------

#define WIZE_CHANNEL            CHANNEL_04                  // Wize channel to use (nodes must be sending on the same channel)
#define WIZE_DATARATE           DATARATE_2400bps            // Wize datarate to use (nodes must be using the same datarate)

//------------------------------------------------------------------------------
// LoRaWAN server configuration
//------------------------------------------------------------------------------

#define LORAWAN_SERVER          IPAddress(52,169,76,203)    // LoRaWAN server (The Things Network EU server)
#define LORAWAN_PORT            1700                        // UDP port
#define LORAWAN_LATITUDE        0.00000000                  // Gateway latitude (degrees, north positive)
#define LORAWAN_LONGITUDE       0.00000000                  // Gateway longitude (degrees, east positive)
#define LORAWAN_ALTITUDE        0                           // Gateway altitude (meters)
#define LORAWAN_GATEWAY_TYPE    "..."                       // Gateway type
#define LORAWAN_EMAIL           "..."                       // Administrator email
#define LORAWAN_DESCRIPTION     "..."                       // Gateway description

//------------------------------------------------------------------------------
// NTP configuration
//------------------------------------------------------------------------------

#define NTP_SERVER              "pool.ntp.org"              // NTP server
#define NTP_SYNC_INTERVAL       10                          // On boot, try to sync every 10 seconds
#define NTP_UPDATE_INTERVAL     1800                        // Once sync'ed, re-sync every 30 minutes
