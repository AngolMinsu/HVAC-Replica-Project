#ifndef HU_HMI_TYPES_H
#define HU_HMI_TYPES_H

#include <Arduino.h>
#include "../can/CanProtocol.h"
#include "../../GDS.h"

enum HuScreen : uint8_t {
  HU_SCREEN_HOME = 0,
  HU_SCREEN_MAP,
  HU_SCREEN_MEDIA,
  HU_SCREEN_HVAC,
  HU_SCREEN_SETTING
};

enum HuPanelFocus : uint8_t {
  HU_PANEL_MEDIA = 0,
  HU_PANEL_MAP,
  HU_PANEL_SETTING
};

enum HuPanelVisualState : uint8_t {
  HU_PANEL_NORMAL = 0,
  HU_PANEL_FOCUSED,
  HU_PANEL_PRESSED
};

enum UiEventType : uint8_t {
  UI_EVENT_NONE = 0,
  UI_EVENT_FOCUS_NEXT,
  UI_EVENT_FOCUS_PREV,
  UI_EVENT_SELECT,
  UI_EVENT_BACK,
  UI_EVENT_HOME,
  UI_EVENT_MENU,
  UI_EVENT_OPEN_MEDIA,
  UI_EVENT_OPEN_MAP,
  UI_EVENT_OPEN_HVAC,
  UI_EVENT_OPEN_SETTING
};

enum AssetEventType : uint8_t {
  ASSET_EVENT_NONE = 0,
  ASSET_EVENT_READY,
  ASSET_EVENT_FAIL
};

enum DirtyFlags : uint32_t {
  DIRTY_NONE = 0,
  DIRTY_TOP_BAR = 1UL << 0,
  DIRTY_BOTTOM_BAR = 1UL << 1,
  DIRTY_HOME = 1UL << 2,
  DIRTY_MEDIA = 1UL << 3,
  DIRTY_HVAC = 1UL << 4,
  DIRTY_MAP = 1UL << 5,
  DIRTY_SETTING = 1UL << 6,
  DIRTY_FOCUS = 1UL << 7,
  DIRTY_STATUS = 1UL << 8,
  DIRTY_FULL = 0xFFFFFFFFUL
};

struct HuStateEvent {
  uint16_t canId;
  CanPayload payload;
  uint32_t rxTick;
};

struct HuCanCommand {
  uint8_t service;
  uint8_t signal;
  uint8_t value;
};

struct UiEvent {
  UiEventType type;
  uint8_t value;
};

struct AssetReadyEvent {
  AssetEventType type;
};

struct MediaInfo {
  char title[32];
  char artist[32];
  char albumAsset[48];
};

struct SystemState {
  HuScreen screen;
  HuPanelFocus focusedPanel;
  HuPanelVisualState panelVisualState;
  uint32_t panelPressedUntilMs;

  uint8_t power;
  uint8_t fanSpeed;
  uint8_t driverTemp;
  uint8_t passengerTemp;
  uint8_t windMode;
  uint8_t ac;
  uint8_t autoMode;
  uint8_t screenMode;
  uint8_t mediaReady;
  uint8_t volume;
  uint8_t mapReady;
  uint8_t mute;
  uint8_t homeReady;
  uint8_t mediaMode;
  uint8_t mediaIndex;

  uint8_t canReady;
  uint8_t canRxOk;
  uint16_t lastCanId;
  uint8_t lastSignal;
  uint8_t lastValue;
  uint32_t lastCanRxMs;

  uint8_t assetsReady;
  uint32_t dirtyFlags;
  uint32_t fps;
  uint32_t freeHeap;
  uint32_t frameCounter;
  uint32_t lastFpsMs;

  MediaInfo media;
};

void initSystemState(SystemState& state);
void applyCanPayloadToState(SystemState& state, const CanPayload& payload);
const char* huScreenToText(HuScreen screen);
const char* huWindToText(uint8_t wind);
uint32_t dirtyForScreen(HuScreen screen);

#endif
