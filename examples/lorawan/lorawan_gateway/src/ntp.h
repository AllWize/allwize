/*

NTP MODULE

*/

#pragma once

#include <TimeLib.h>
#include <NtpClientLib.h>

bool ntpSynced();
String ntpDateTime(time_t t);
String ntpDateTime();
void ntpConnect();
void ntpSetup();
void ntpLoop();