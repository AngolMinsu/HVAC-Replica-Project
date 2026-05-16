#include "MapScreen.h"

void drawMapScreen(TFT_eSPI& tft, AssetManager& assets, const SystemState& state) {
  (void)state;
  assets.drawFallbackPanel(tft, 24, 58, 432, 190, "MAP VIEW", 0x5D9F);
  tft.drawLine(76, 194, 194, 126, 0x5D9F);
  tft.drawLine(194, 126, 318, 170, 0x5D9F);
  tft.fillCircle(76, 194, 5, TFT_WHITE);
  tft.fillCircle(318, 170, 6, 0xFBE0);
}

