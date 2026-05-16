#include "MediaScreen.h"

void drawMediaScreen(TFT_eSPI& tft, AssetManager& assets, const SystemState& state) {
  tft.fillRoundRect(24, 62, 176, 176, 14, 0x0824);
  if (!assets.drawPng(tft, state.media.albumAsset, 42, 78)) {
    assets.drawFallbackPanel(tft, 42, 78, 140, 140, "ALBUM", 0x5D9F);
  }

  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("MEDIA", 230, 70, 4);
  tft.drawString(state.media.title, 230, 122, 4);
  tft.setTextColor(0x9CF3);
  tft.drawString(state.media.artist, 230, 160, 2);

  tft.drawRoundRect(230, 204, 190, 10, 5, 0x294A);
  tft.fillRoundRect(230, 204, map(state.mediaIndex, 0, 30, 8, 180), 10, 5, 0x5D9F);

  char volume[32];
  snprintf(volume, sizeof(volume), "%s  VOL %02u", state.mute ? "MUTE" : "PLAY", state.volume);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(volume, 230, 230, 2);
}

