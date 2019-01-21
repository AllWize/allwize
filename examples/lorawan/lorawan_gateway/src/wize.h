/*

WIZE MODULE

*/

#pragma once

#include "AllWize.h"

void wizeSetup();
void wizeDebugMessage(allwize_message_t message);
double wizeFrequency(uint8_t channel);
uint16_t wizeDataRateSpeed(uint8_t dr);
void wizeLoop();