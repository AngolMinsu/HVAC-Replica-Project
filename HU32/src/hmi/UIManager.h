#ifndef HU_UI_MANAGER_H
#define HU_UI_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "AssetManager.h"
#include "HuTypes.h"

class UIManager {
public:
  void begin(AssetManager* assetManager);
  void render(const SystemState& state, uint32_t dirtyFlags);
  void renderPressedOverlay(const SystemState& state);

private:
  TFT_eSPI tft;
  AssetManager* assets;
  HuScreen currentScreen;
  bool initialized;

  void drawTopBar(const SystemState& state);
  void drawBottomBar(const SystemState& state);
  void drawScreen(const SystemState& state, uint32_t dirtyFlags);
  void drawHome(const SystemState& state, uint32_t dirtyFlags);
  void drawMedia(const SystemState& state);
  void drawHvac(const SystemState& state);
  void drawMap(const SystemState& state);
  void drawSetting(const SystemState& state);
  void drawTextOverBars(const SystemState& state);
  bool drawAsset(const char* path, int16_t x, int16_t y, int16_t w, int16_t h, const char* fallback, uint16_t borderColor);
};

#endif
