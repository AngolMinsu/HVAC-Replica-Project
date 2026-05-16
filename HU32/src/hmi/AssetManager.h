#ifndef HU_ASSET_MANAGER_H
#define HU_ASSET_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

class AssetManager {
public:
  bool begin();
  bool isReady() const;
  bool drawPng(TFT_eSPI& tft, const char* path, int16_t x, int16_t y);
  void drawFallbackPanel(TFT_eSPI& tft, int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t borderColor);

private:
  bool ready;
};

#endif
