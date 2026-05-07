#ifndef INFO_H
#define INFO_H

#include <U8g2lib.h>
#include "../state/State.h"

uint8_t infoHandleButton2(SystemState& state);
uint8_t infoHandleButton3(SystemState& state);
uint8_t infoHandleButton4(SystemState& state);
uint8_t infoHandleButton5(SystemState& state);

void infoDrawScreen(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, const SystemState& state);

#endif
