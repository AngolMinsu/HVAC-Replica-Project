#ifndef HU_HOME_SCREEN_H
#define HU_HOME_SCREEN_H

#include <TFT_eSPI.h>
#include "../AssetManager.h"
#include "../HuTypes.h"

void drawHomeScreen(TFT_eSPI& tft, AssetManager& assets, const SystemState& state);

#endif
