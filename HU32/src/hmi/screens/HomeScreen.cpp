#include "HomeScreen.h"

static const uint16_t COLOR_BG = 0x0008;
static const uint16_t COLOR_GLOW = 0x5D9F;
static const uint16_t COLOR_TEXT = TFT_WHITE;
static const uint16_t COLOR_MUTED = 0x9CF3;

static const char* panelAsset(HuPanelFocus panel, HuPanelFocus focus, HuPanelVisualState visual) {
  const char* state = "normal";
  if (panel == focus) {
    state = visual == HU_PANEL_PRESSED ? "pressed" : "focused";
  }

  switch (panel) {
    case HU_PANEL_MEDIA:
      if (strcmp(state, "pressed") == 0) return "/assets/media_panel_pressed.png";
      if (strcmp(state, "focused") == 0) return "/assets/media_panel_focused.png";
      return "/assets/media_panel_normal.png";
    case HU_PANEL_MAP:
      if (strcmp(state, "pressed") == 0) return "/assets/map_panel_pressed.png";
      if (strcmp(state, "focused") == 0) return "/assets/map_panel_focused.png";
      return "/assets/map_panel_normal.png";
    case HU_PANEL_SETTING:
      if (strcmp(state, "pressed") == 0) return "/assets/setting_panel_pressed.png";
      if (strcmp(state, "focused") == 0) return "/assets/setting_panel_focused.png";
      return "/assets/setting_panel_normal.png";
  }

  return "/assets/media_panel_normal.png";
}

static void drawPanel(TFT_eSPI& tft, AssetManager& assets, HuPanelFocus panel, const SystemState& state, int16_t x, int16_t y, const char* title, const char* line1, const char* line2) {
  int16_t offset = (panel == state.focusedPanel && state.panelVisualState == HU_PANEL_PRESSED) ? 1 : 0;
  const uint16_t w = 142;
  const uint16_t h = 180;

  if (!assets.drawPng(tft, panelAsset(panel, state.focusedPanel, state.panelVisualState), x + offset, y + offset)) {
    assets.drawFallbackPanel(tft, x + offset, y + offset, w, h, title, panel == state.focusedPanel ? COLOR_GLOW : 0x294A);
  }

  if (panel == state.focusedPanel) {
    tft.drawRoundRect(x - 1 + offset, y - 1 + offset, w + 2, h + 2, 12, COLOR_GLOW);
  }

  if (panel == state.focusedPanel && state.panelVisualState == HU_PANEL_PRESSED) {
    tft.fillRoundRect(x + 4, y + 4, w - 8, h - 8, 10, 0x8410);
  }

  tft.setTextColor(COLOR_TEXT);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(title, x + 16 + offset, y + 18 + offset, 2);
  tft.setTextColor(COLOR_MUTED);
  tft.drawString(line1, x + 16 + offset, y + 124 + offset, 2);
  tft.drawString(line2, x + 16 + offset, y + 146 + offset, 2);
}

void drawHomeScreen(TFT_eSPI& tft, AssetManager& assets, const SystemState& state) {
  tft.fillRect(0, 40, 480, 240, COLOR_BG);

  char mediaLine[32];
  snprintf(mediaLine, sizeof(mediaLine), "%s %02u", state.mediaMode ? "PLAYLIST" : "MEDIA", state.mediaIndex);

  char mapLine[32];
  snprintf(mapLine, sizeof(mapLine), "MAP %s", state.mapReady ? "READY" : "STANDBY");

  char hvacLine[32];
  snprintf(hvacLine, sizeof(hvacLine), "DRV %u  PSG %u", state.driverTemp, state.passengerTemp);

  drawPanel(tft, assets, HU_PANEL_MEDIA, state, 18, 60, "MEDIA", state.media.title, mediaLine);
  drawPanel(tft, assets, HU_PANEL_MAP, state, 169, 60, "MAP", mapLine, "Route preview");
  drawPanel(tft, assets, HU_PANEL_SETTING, state, 320, 60, "SETTING", hvacLine, "Vehicle status");
}

