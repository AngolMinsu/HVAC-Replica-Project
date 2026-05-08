#ifndef CAN_MONITOR_H
#define CAN_MONITOR_H

#include <Arduino.h>
#include "CanDriver.h"

void canMonitorPrintFrame(const char* direction, const CanFrame& frame);

#endif
