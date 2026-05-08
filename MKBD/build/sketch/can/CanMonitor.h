#line 1 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\can\\CanMonitor.h"
#ifndef CAN_MONITOR_H
#define CAN_MONITOR_H

#include <Arduino.h>
#include "CanDriver.h"

void canMonitorPrintFrame(const char* direction, const CanFrame& frame);

#endif
