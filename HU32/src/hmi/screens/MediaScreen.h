#ifndef HU_MEDIA_SCREEN_H
#define HU_MEDIA_SCREEN_H

#include <TFT_eSPI.h>
#include "../AssetManager.h"
#include "../HuTypes.h"

void drawMediaScreen(TFT_eSPI& tft, AssetManager& assets, const SystemState& state);

#endif
