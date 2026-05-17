#include "UIManager.h"

#include "screens/HomeScreen.h"
#include "screens/HvacScreen.h"
#include "screens/MapScreen.h"
#include "screens/MediaScreen.h"
#include "screens/SettingScreen.h"

static const uint16_t COLOR_BG = 0x0008;
static const uint16_t COLOR_PANEL = 0x0824;
static const uint16_t COLOR_TEXT = TFT_WHITE;
static const uint16_t COLOR_MUTED = 0x9CF3;
static const uint16_t COLOR_ACCENT = 0x5D9F;
static const uint16_t TOP_H = 40;
static const uint16_t BOTTOM_H = 40;

void UIManager::begin(AssetManager* assetManager) {
  assets = assetManager;
  Serial.println("TFT init start");
  Serial.print("TFT driver ST7796, MOSI:");
  Serial.print(TFT_MOSI);
  Serial.print(" SCLK:");
  Serial.print(TFT_SCLK);
  Serial.print(" CS:");
  Serial.print(TFT_CS);
  Serial.print(" DC:");
  Serial.print(TFT_DC);
  Serial.print(" RST:");
  Serial.println(TFT_RST);
  tft.init();
  tft.setRotation(GDS_TFT_ROTATION);
  tft.fillScreen(COLOR_BG);
  tft.setTextFont(2);
  tft.setTextDatum(TL_DATUM);
  currentScreen = HU_SCREEN_HOME;
  initialized = true;
  Serial.println("TFT init done");
}

void UIManager::render(const SystemState& state, uint32_t dirtyFlags) {
  if (!initialized || dirtyFlags == DIRTY_NONE) {
    return;
  }

  if (dirtyFlags == DIRTY_FULL || currentScreen != state.screen) {
    tft.fillScreen(COLOR_BG);
    dirtyFlags = DIRTY_FULL;
    currentScreen = state.screen;
  }

  if (dirtyFlags & (DIRTY_FULL | DIRTY_TOP_BAR | DIRTY_STATUS)) {
    drawTopBar(state);
  }

  drawScreen(state, dirtyFlags);

  if (dirtyFlags & (DIRTY_FULL | DIRTY_BOTTOM_BAR | DIRTY_STATUS)) {
    drawBottomBar(state);
  }
}

void UIManager::renderPressedOverlay(const SystemState& state) {
  if (state.screen == HU_SCREEN_HOME) {
    drawHome(state, DIRTY_FOCUS);
  }
}

void UIManager::drawTopBar(const SystemState& state) {
  if (!drawAsset("/assets/topbar_base.png", 0, 0, 480, TOP_H, "", COLOR_PANEL)) {
    tft.fillRect(0, 0, 480, TOP_H, 0x020A);
  }

  drawTextOverBars(state);
}

void UIManager::drawBottomBar(const SystemState& state) {
  if (!drawAsset("/assets/bottombar_base.png", 0, 280, 480, BOTTOM_H, "", COLOR_PANEL)) {
    tft.fillRect(0, 280, 480, BOTTOM_H, 0x020A);
  }

  drawAsset("/assets/back_normal.png", 10, 286, 28, 28, "<", COLOR_ACCENT);
  drawAsset("/assets/home_normal.png", 52, 286, 28, 28, "H", COLOR_ACCENT);
  drawAsset("/assets/hamburger_normal.png", 94, 286, 28, 28, "M", COLOR_ACCENT);

  tft.setTextColor(COLOR_TEXT, 0x020A);
  tft.setTextDatum(MC_DATUM);
  char info[48];
  if (state.screen == HU_SCREEN_MAP) {
    snprintf(info, sizeof(info), "Route 12.4km  ETA 18:42");
  } else {
    snprintf(info, sizeof(info), "DRV %u  PSG %u  FAN %u  %s", state.driverTemp, state.passengerTemp, state.fanSpeed, huWindToText(state.windMode));
  }
  tft.drawString(info, 250, 300, 2);

  char media[36];
  snprintf(media, sizeof(media), "%s %02u", state.mute ? "MUTE" : "VOL", state.mute ? 0 : state.volume);
  tft.setTextDatum(MR_DATUM);
  tft.drawString(media, 470, 300, 2);
  tft.setTextDatum(TL_DATUM);
}

void UIManager::drawScreen(const SystemState& state, uint32_t dirtyFlags) {
  switch (state.screen) {
    case HU_SCREEN_HOME: drawHome(state, dirtyFlags); break;
    case HU_SCREEN_MEDIA: drawMedia(state); break;
    case HU_SCREEN_HVAC: drawHvac(state); break;
    case HU_SCREEN_MAP: drawMap(state); break;
    case HU_SCREEN_SETTING: drawSetting(state); break;
  }
}

void UIManager::drawHome(const SystemState& state, uint32_t dirtyFlags) {
  if (dirtyFlags & (DIRTY_FULL | DIRTY_HOME)) {
    tft.fillRect(0, TOP_H, 480, 240, COLOR_BG);
  }
  drawHomeScreen(tft, *assets, state);
}

void UIManager::drawMedia(const SystemState& state) {
  tft.fillRect(0, TOP_H, 480, 240, COLOR_BG);
  drawMediaScreen(tft, *assets, state);
}

void UIManager::drawHvac(const SystemState& state) {
  tft.fillRect(0, TOP_H, 480, 240, COLOR_BG);
  drawHvacScreen(tft, state);
}

void UIManager::drawMap(const SystemState& state) {
  tft.fillRect(0, TOP_H, 480, 240, COLOR_BG);
  drawMapScreen(tft, *assets, state);
}

void UIManager::drawSetting(const SystemState& state) {
  tft.fillRect(0, TOP_H, 480, 240, COLOR_BG);
  drawSettingScreen(tft, state);
}

void UIManager::drawTextOverBars(const SystemState& state) {
  tft.setTextColor(COLOR_TEXT, 0x020A);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(huScreenToText(state.screen), 16, 20, 2);

  uint32_t sec = millis() / 1000;
  uint16_t day = (sec / 86400) % 7;
  const char* days[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  char date[32];
  snprintf(date, sizeof(date), "%s 2026.05.16", days[day]);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(date, 240, 20, 2);

  char time[16];
  snprintf(time, sizeof(time), "%02lu:%02lu", (sec / 3600) % 24, (sec / 60) % 60);
  tft.setTextDatum(MR_DATUM);
  tft.drawString(time, 464, 20, 2);

  tft.setTextDatum(TL_DATUM);
}

bool UIManager::drawAsset(const char* path, int16_t x, int16_t y, int16_t w, int16_t h, const char* fallback, uint16_t borderColor) {
  if (assets && assets->drawPng(tft, path, x, y)) {
    return true;
  }

  if (fallback && fallback[0] != '\0') {
    assets->drawFallbackPanel(tft, x, y, w, h, fallback, borderColor);
  }
  return false;
}
