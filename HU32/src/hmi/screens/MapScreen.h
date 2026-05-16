#ifndef HU_MAP_SCREEN_H
#define HU_MAP_SCREEN_H

#include <TFT_eSPI.h>
#include "../AssetManager.h"
#include "../HuTypes.h"

void drawMapScreen(TFT_eSPI& tft, AssetManager& assets, const SystemState& state);

#endif
