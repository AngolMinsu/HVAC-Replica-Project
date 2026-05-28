#ifndef CAN_MONITOR_H
#define CAN_MONITOR_H

#include "CanDriver.h"

void canMonitorPrintFrame(const char* label, const CanFrame& frame);

#endif
