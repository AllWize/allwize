/*

WIFI MODULE

*/

#include "wifi.h"
#include "ntp.h"
#include "debug.h"
#include "configuration.h"

#include <ESP8266WiFi.h>
#include <Ticker.h>

WiFiEventHandler _wifi_connect_handler;
WiFiEventHandler _wifi_disconnect_handler;
Ticker _wifi_timer;

bool wifiConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

void wifiConnect() {
    DEBUG_MSG("[WIFI] Connecting to '%s'\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void _wifiOnConnect(const WiFiEventStationModeGotIP& event) {
    DEBUG_MSG("[WIFI] Connected!\n");
    _wifi_timer.detach();
    _wifi_timer.once(2, ntpConnect);
}

void _wifiOnDisconnect(const WiFiEventStationModeDisconnected& event) {
    DEBUG_MSG("[WIFI] Disconnected!\n");
    _wifi_timer.detach();
    _wifi_timer.once(2, wifiConnect);
}

void wifiSetup() {
    _wifi_connect_handler = WiFi.onStationModeGotIP(_wifiOnConnect);
    _wifi_disconnect_handler = WiFi.onStationModeDisconnected(_wifiOnDisconnect);
    WiFi.persistent(false);
    WiFi.disconnect();
    delay(1);
}