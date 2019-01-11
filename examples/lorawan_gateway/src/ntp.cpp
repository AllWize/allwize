/*

NTP MODULE

*/

#include "ntp.h"
#include "debug.h"
#include "configuration.h"

#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ESP8266WiFi.h>

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

bool ntpSynced() {
    return (NTP.getLastNTPSync() > 0);
}

String ntpDateTime(time_t t) {
    char buffer[30];
    snprintf_P(buffer, sizeof(buffer),
        PSTR("%04d-%02d-%02d %02d:%02d:%02d"),
        year(t), month(t), day(t), hour(t), minute(t), second(t)
    );
    return String(buffer);
}

String ntpDateTime() {
    if (ntpSynced()) return ntpDateTime(now());
    return String();
}

void ntpConnect() {
    NTP.begin(NTP_SERVER);
    NTP.setInterval(NTP_SYNC_INTERVAL, NTP_UPDATE_INTERVAL);
}

void ntpSetup() {

    NTP.onNTPSyncEvent([](NTPSyncEvent_t error) {
        if (error) {
            if (error == noResponse) {
                DEBUG_MSG("[NTP] Warning: NTP server not reachable\n");
            } else if (error == invalidAddress) {
                DEBUG_MSG("[NTP] Error: Invalid NTP server address\n");
            }
        } else {
            DEBUG_MSG("[NTP] Current time: %s\n", NTP.getTimeDateString(NTP.getFirstSync()).c_str());
        }
    });

}

void ntpLoop() {
    now();
}