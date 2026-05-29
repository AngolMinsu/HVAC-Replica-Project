#ifndef CAN_MONITOR_H
#define CAN_MONITOR_H

#include "CanDriver.h"
#include "CanProtocol.h"

void canMonitorPrintFrame(const char* label, const CanFrame& frame);
void canMonitorPrintPayloadSummary(const CanPayload& payload);

#endif
