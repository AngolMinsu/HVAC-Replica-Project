#ifndef DATC_H
#define DATC_H

#include <U8g2lib.h>
#include "../state/State.h"

uint8_t datcIncreaseDriverTemp(SystemState& state);
uint8_t datcDecreaseDriverTemp(SystemState& state);
uint8_t datcIncreasePassengerTemp(SystemState& state);
uint8_t datcDecreasePassengerTemp(SystemState& state);
uint8_t datcIncreaseFanSpeed(SystemState& state);
uint8_t datcDecreaseFanSpeed(SystemState& state);
uint8_t datcCycleWindMode(SystemState& state);

void datcDrawScreen(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, const SystemState& state);

#endif
