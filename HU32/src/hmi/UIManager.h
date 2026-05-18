#ifndef HU_UI_MANAGER_H
#define HU_UI_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include "AssetManager.h"
#include "HuTypes.h"

class UIManager {
public:
  void begin(AssetManager* assetManager);
  void loop();
  void render(const SystemState& state, uint32_t dirtyFlags);
  void renderPressedOverlay(const SystemState& state);

private:
  TFT_eSPI tft;
  AssetManager* assets;
  HuScreen currentScreen;
  bool initialized;
  uint32_t lastLvglTickMs;

  void buildLayout(const SystemState& state);
  void buildTopBar(lv_obj_t* parent, const SystemState& state);
  void buildBottomBar(lv_obj_t* parent, const SystemState& state);
  void buildHome(lv_obj_t* parent, const SystemState& state);
  void buildMedia(lv_obj_t* parent, const SystemState& state);
  void buildHvac(lv_obj_t* parent, const SystemState& state);
  void buildMap(lv_obj_t* parent, const SystemState& state);
  void buildSetting(lv_obj_t* parent, const SystemState& state);
};

#endif
