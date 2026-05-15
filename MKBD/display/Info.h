#ifndef INFO_H
#define INFO_H

#include <U8g2lib.h>
#include "../state/State.h"

uint8_t infoIncreaseVolume(SystemState& state);
uint8_t infoDecreaseVolume(SystemState& state);
uint8_t infoToggleMute(SystemState& state);
uint8_t infoClearMute(SystemState& state);
uint8_t infoTuneUp(SystemState& state);
uint8_t infoTuneDown(SystemState& state);
uint8_t infoSelect(SystemState& state);
uint8_t infoHandleMap(SystemState& state);
uint8_t infoHandleNav(SystemState& state);
uint8_t infoHandleRadio(SystemState& state);

void infoDrawScreen(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, const SystemState& state);

#endif
