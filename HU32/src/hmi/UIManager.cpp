#include "UIManager.h"

static TFT_eSPI* activeTft = nullptr;
static lv_disp_draw_buf_t drawBuffer;
static lv_color_t lvBuffer[GDS_TFT_WIDTH * 20];

static const lv_color_t COLOR_BG = lv_color_hex(0x07111F);
static const lv_color_t COLOR_BAR = lv_color_hex(0x0B1726);
static const lv_color_t COLOR_PANEL = lv_color_hex(0x122033);
static const lv_color_t COLOR_FOCUS = lv_color_hex(0x4AA3FF);
static const lv_color_t COLOR_TEXT = lv_color_hex(0xF4F7FB);
static const lv_color_t COLOR_MUTED = lv_color_hex(0x8FA6C1);
static const uint16_t TOP_H = 40;
static const uint16_t BOTTOM_H = 40;

static void displayFlush(lv_disp_drv_t* display, const lv_area_t* area, lv_color_t* colors) {
  (void)display;
  if (!activeTft) {
    lv_disp_flush_ready(display);
    return;
  }

  uint32_t width = area->x2 - area->x1 + 1;
  uint32_t height = area->y2 - area->y1 + 1;
  activeTft->startWrite();
  activeTft->setAddrWindow(area->x1, area->y1, width, height);
  activeTft->pushColors((uint16_t*)&colors->full, width * height, true);
  activeTft->endWrite();
  lv_disp_flush_ready(display);
}

static lv_obj_t* makePanel(lv_obj_t* parent, int16_t x, int16_t y, int16_t w, int16_t h, bool focused, bool pressed = false) {
  lv_obj_t* panel = lv_obj_create(parent);
  lv_obj_set_pos(panel, x, y);
  lv_obj_set_size(panel, w, h);
  lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_radius(panel, 10, 0);
  lv_obj_set_style_bg_color(panel, pressed ? lv_color_hex(0xEAF3FF) : COLOR_PANEL, 0);
  lv_obj_set_style_bg_opa(panel, pressed ? LV_OPA_70 : LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(panel, focused ? 2 : 1, 0);
  lv_obj_set_style_border_color(panel, pressed ? lv_color_hex(0xFFFFFF) : (focused ? COLOR_FOCUS : lv_color_hex(0x23354D)), 0);
  lv_obj_set_style_pad_all(panel, 12, 0);
  if (pressed) {
    lv_obj_set_style_translate_y(panel, 1, 0);
  }
  return panel;
}

static lv_obj_t* makeLabel(lv_obj_t* parent, const char* text, lv_color_t color, int16_t x, int16_t y) {
  lv_obj_t* label = lv_label_create(parent);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_color(label, color, 0);
  lv_obj_set_pos(label, x, y);
  return label;
}

static void makeValueRow(lv_obj_t* parent, const char* name, const char* value, int16_t y) {
  makeLabel(parent, name, COLOR_MUTED, 0, y);
  lv_obj_t* valueLabel = makeLabel(parent, value, COLOR_TEXT, 92, y);
  lv_obj_set_style_text_align(valueLabel, LV_TEXT_ALIGN_RIGHT, 0);
}

static lv_obj_t* makeSettingTile(lv_obj_t* parent, int16_t x, int16_t y, const char* title, const char* detail, bool focused) {
  lv_obj_t* tile = makePanel(parent, x, y, 118, 78, focused);
  lv_obj_set_style_radius(tile, 8, 0);
  lv_obj_set_style_pad_all(tile, 8, 0);

  lv_obj_t* titleLabel = makeLabel(tile, title, COLOR_TEXT, 0, 0);
  lv_obj_set_width(titleLabel, 100);
  lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_WRAP);

  lv_obj_t* detailLabel = makeLabel(tile, detail, COLOR_MUTED, 0, 44);
  lv_obj_set_width(detailLabel, 100);
  lv_label_set_long_mode(detailLabel, LV_LABEL_LONG_WRAP);
  return tile;
}

void UIManager::begin(AssetManager* assetManager) {
  assets = assetManager;
  tft.init();
  tft.setRotation(GDS_TFT_ROTATION);
  tft.fillScreen(TFT_BLACK);
  activeTft = &tft;

  lv_init();
  lv_disp_draw_buf_init(&drawBuffer, lvBuffer, nullptr, GDS_TFT_WIDTH * 20);

  static lv_disp_drv_t displayDriver;
  lv_disp_drv_init(&displayDriver);
  displayDriver.hor_res = GDS_TFT_WIDTH;
  displayDriver.ver_res = GDS_TFT_HEIGHT;
  displayDriver.flush_cb = displayFlush;
  displayDriver.draw_buf = &drawBuffer;
  lv_disp_drv_register(&displayDriver);

  currentScreen = HU_SCREEN_HOME;
  initialized = true;
  lastLvglTickMs = millis();
}

void UIManager::loop() {
  if (!initialized) {
    return;
  }

  uint32_t now = millis();
  lv_tick_inc(now - lastLvglTickMs);
  lastLvglTickMs = now;
  lv_timer_handler();
}

void UIManager::render(const SystemState& state, uint32_t dirtyFlags) {
  if (!initialized || dirtyFlags == DIRTY_NONE) {
    return;
  }

  buildLayout(state);
  currentScreen = state.screen;
}

void UIManager::renderPressedOverlay(const SystemState& state) {
  if (!initialized) {
    return;
  }

  buildLayout(state);
}

void UIManager::buildLayout(const SystemState& state) {
  lv_obj_t* screen = lv_scr_act();
  lv_obj_clean(screen);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(screen, COLOR_BG, 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  buildTopBar(screen, state);

  switch (state.screen) {
    case HU_SCREEN_HOME: buildHome(screen, state); break;
    case HU_SCREEN_MEDIA: buildMedia(screen, state); break;
    case HU_SCREEN_HVAC: buildHvac(screen, state); break;
    case HU_SCREEN_MAP: buildMap(screen, state); break;
    case HU_SCREEN_SETTING: buildSetting(screen, state); break;
  }

  buildBottomBar(screen, state);
}

void UIManager::buildTopBar(lv_obj_t* parent, const SystemState& state) {
  lv_obj_t* bar = lv_obj_create(parent);
  lv_obj_set_pos(bar, 0, 0);
  lv_obj_set_size(bar, 480, TOP_H);
  lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_radius(bar, 0, 0);
  lv_obj_set_style_bg_color(bar, COLOR_BAR, 0);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_set_style_pad_all(bar, 0, 0);

  makeLabel(bar, huScreenToText(state.screen), COLOR_TEXT, 16, 11);

  uint32_t sec = millis() / 1000;
  char date[36];
  snprintf(date, sizeof(date), "2026.05.18");
  lv_obj_t* center = makeLabel(bar, date, COLOR_MUTED, 170, 11);
  lv_obj_set_width(center, 140);
  lv_obj_set_style_text_align(center, LV_TEXT_ALIGN_CENTER, 0);

  char timeText[16];
  snprintf(timeText, sizeof(timeText), "%02lu:%02lu", (sec / 3600) % 24, (sec / 60) % 60);
  lv_obj_t* right = makeLabel(bar, timeText, COLOR_TEXT, 360, 11);
  lv_obj_set_width(right, 104);
  lv_obj_set_style_text_align(right, LV_TEXT_ALIGN_RIGHT, 0);
}

void UIManager::buildBottomBar(lv_obj_t* parent, const SystemState& state) {
  lv_obj_t* bar = lv_obj_create(parent);
  lv_obj_set_pos(bar, 0, 280);
  lv_obj_set_size(bar, 480, BOTTOM_H);
  lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_radius(bar, 0, 0);
  lv_obj_set_style_bg_color(bar, COLOR_BAR, 0);
  lv_obj_set_style_border_width(bar, 0, 0);
  lv_obj_set_style_pad_all(bar, 0, 0);

  makeLabel(bar, "<", COLOR_TEXT, 18, 11);
  makeLabel(bar, "HOME", COLOR_TEXT, 54, 11);
  makeLabel(bar, "MENU", COLOR_TEXT, 112, 11);

  char info[64];
  if (state.screen == HU_SCREEN_MAP) {
    snprintf(info, sizeof(info), "Route 12.4km  ETA 18:42");
  } else {
    snprintf(info, sizeof(info), "DRV %u  PSG %u  FAN %u  %s", state.driverTemp, state.passengerTemp, state.fanSpeed, huWindToText(state.windMode));
  }
  lv_obj_t* center = makeLabel(bar, info, COLOR_MUTED, 180, 11);
  lv_obj_set_width(center, 180);
  lv_obj_set_style_text_align(center, LV_TEXT_ALIGN_CENTER, 0);

  char media[28];
  snprintf(media, sizeof(media), "%s %02u", state.mute ? "MUTE" : "VOL", state.mute ? 0 : state.volume);
  lv_obj_t* right = makeLabel(bar, media, COLOR_TEXT, 395, 11);
  lv_obj_set_width(right, 70);
  lv_obj_set_style_text_align(right, LV_TEXT_ALIGN_RIGHT, 0);
}

void UIManager::buildHome(lv_obj_t* parent, const SystemState& state) {
  bool mediaFocused = state.focusedPanel == HU_PANEL_MEDIA;
  bool mapFocused = state.focusedPanel == HU_PANEL_MAP;
  bool settingFocused = state.focusedPanel == HU_PANEL_SETTING;
  bool mediaPressed = mediaFocused && state.panelVisualState == HU_PANEL_PRESSED;
  bool mapPressed = mapFocused && state.panelVisualState == HU_PANEL_PRESSED;
  bool settingPressed = settingFocused && state.panelVisualState == HU_PANEL_PRESSED;
  lv_color_t mediaText = mediaPressed ? COLOR_BG : COLOR_TEXT;
  lv_color_t mediaSub = mediaPressed ? lv_color_hex(0x223048) : COLOR_MUTED;
  lv_color_t mapText = mapPressed ? COLOR_BG : COLOR_TEXT;
  lv_color_t mapSub = mapPressed ? lv_color_hex(0x223048) : COLOR_MUTED;
  lv_color_t settingText = settingPressed ? COLOR_BG : COLOR_TEXT;
  lv_color_t settingSub = settingPressed ? lv_color_hex(0x223048) : COLOR_MUTED;

  lv_obj_t* media = makePanel(parent, 18, 60, 142, 180, mediaFocused, mediaPressed);
  makeLabel(media, "MEDIA", mediaText, 0, 0);
  makeLabel(media, state.media.title, mediaSub, 0, 92);
  char mediaLine[28];
  snprintf(mediaLine, sizeof(mediaLine), "Track %02u", state.mediaIndex);
  makeLabel(media, mediaLine, mediaSub, 0, 120);

  lv_obj_t* map = makePanel(parent, 169, 60, 142, 180, mapFocused, mapPressed);
  makeLabel(map, "MAP", mapText, 0, 0);
  makeLabel(map, state.mapReady ? "Route ready" : "Map standby", mapSub, 0, 92);
  makeLabel(map, "MKBD encoder", mapSub, 0, 120);

  lv_obj_t* setting = makePanel(parent, 320, 60, 142, 180, settingFocused, settingPressed);
  makeLabel(setting, "SETTING", settingText, 0, 0);
  makeLabel(setting, "Vehicle status", settingSub, 0, 92);
  makeLabel(setting, "System info", settingSub, 0, 120);
}

void UIManager::buildMedia(lv_obj_t* parent, const SystemState& state) {
  lv_obj_t* panel = makePanel(parent, 24, 58, 432, 190, true);
  makeLabel(panel, "MEDIA", COLOR_TEXT, 0, 0);
  makeLabel(panel, state.media.title, COLOR_TEXT, 0, 52);
  makeLabel(panel, state.media.artist, COLOR_MUTED, 0, 82);

  char line[40];
  snprintf(line, sizeof(line), "Index %02u / Volume %02u", state.mediaIndex, state.volume);
  makeLabel(panel, line, COLOR_MUTED, 0, 122);
}

void UIManager::buildHvac(lv_obj_t* parent, const SystemState& state) {
  lv_obj_t* panel = makePanel(parent, 24, 58, 432, 190, true);
  makeLabel(panel, "HVAC", COLOR_TEXT, 0, 0);

  char value[20];
  snprintf(value, sizeof(value), "%u", state.fanSpeed);
  makeValueRow(panel, "FAN", value, 44);
  snprintf(value, sizeof(value), "%u", state.driverTemp);
  makeValueRow(panel, "DRV", value, 76);
  snprintf(value, sizeof(value), "%u", state.passengerTemp);
  makeValueRow(panel, "PSG", value, 108);
  makeValueRow(panel, "AIR", huWindToText(state.windMode), 140);
}

void UIManager::buildMap(lv_obj_t* parent, const SystemState& state) {
  lv_obj_t* panel = makePanel(parent, 24, 58, 432, 190, true);
  makeLabel(panel, "HOME / MAP", COLOR_TEXT, 0, 0);
  makeLabel(panel, state.mapReady ? "Map data ready" : "Waiting for map signal", COLOR_MUTED, 0, 52);
  makeLabel(panel, "480x320 non-touch HU display", COLOR_MUTED, 0, 92);
}

void UIManager::buildSetting(lv_obj_t* parent, const SystemState& state) {
  (void)state;
  makeLabel(parent, "SETTING", COLOR_TEXT, 24, 50);

  makeSettingTile(parent, 36, 82, "Device", "HU32 info", state.focusedSettingTile == 0);
  makeSettingTile(parent, 181, 82, "Wi-Fi", "AP / Connect", state.focusedSettingTile == 1);
  makeSettingTile(parent, 326, 82, "Connect", "Devices", state.focusedSettingTile == 2);
  makeSettingTile(parent, 36, 174, "Profile", "User", state.focusedSettingTile == 3);
  makeSettingTile(parent, 181, 174, "Button", "MKBD input", state.focusedSettingTile == 4);
  makeSettingTile(parent, 326, 174, "General", "UI", state.focusedSettingTile == 5);
}
