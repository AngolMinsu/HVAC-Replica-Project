#ifndef HU_CAN_MONITOR_H
#define HU_CAN_MONITOR_H

#include <Arduino.h>
#include "CanDriver.h"
#include "CanProtocol.h"

void canMonitorPrintFrame(const char* direction, const CanFrame& frame);
void canMonitorPrintPayloadSummary(const CanPayload& payload);

#endif
