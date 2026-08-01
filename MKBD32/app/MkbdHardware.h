#ifndef MKBD_HARDWARE_H
#define MKBD_HARDWARE_H

#include <Arduino.h>

#include "../state/State.h"

void mkbdHardwareBegin();
uint8_t mkbdHardwareReadButtonEvent();
uint8_t mkbdHardwareReadEncoderEvent();
int mkbdHardwareUpdateFan(const SystemState& state);
uint8_t mkbdHardwareDrawDisplay(const SystemState& state);

#endif
