#ifndef HU_HVAC_SCREEN_H
#define HU_HVAC_SCREEN_H

#include <TFT_eSPI.h>
#include "../HuTypes.h"

void drawHvacScreen(TFT_eSPI& tft, const SystemState& state);

#endif
