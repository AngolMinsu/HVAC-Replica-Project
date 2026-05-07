#ifndef DATC_H
#define DATC_H

#include <U8g2lib.h>
#include "../state/State.h"

uint8_t datcHandleButton2(SystemState& state);
uint8_t datcHandleButton3(SystemState& state);
uint8_t datcHandleButton4(SystemState& state);
uint8_t datcHandleButton5(SystemState& state);

void datcDrawScreen(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, const SystemState& state);

#endif
