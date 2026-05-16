#include "HuTypes.h"

void initSystemState(SystemState& state) {
  memset(&state, 0, sizeof(state));
  state.screen = HU_SCREEN_HOME;
  state.focusedPanel = HU_PANEL_MEDIA;
  state.panelVisualState = HU_PANEL_FOCUSED;
  state.driverTemp = 24;
  state.passengerTemp = 24;
  state.volume = 10;
  state.mediaIndex = 0;
  state.dirtyFlags = DIRTY_FULL;
  strncpy(state.media.title, "No Media", sizeof(state.media.title) - 1);
  strncpy(state.media.artist, "HU32", sizeof(state.media.artist) - 1);
  strncpy(state.media.albumAsset, "/assets/media_panel_normal.png", sizeof(state.media.albumAsset) - 1);
}

void applyCanPayloadToState(SystemState& state, const CanPayload& payload) {
  if (!canValidateChecksum(payload) || payload.result == CAN_RESULT_FAIL) {
    return;
  }

  state.lastSignal = payload.signal;
  state.lastValue = payload.value;

  switch (payload.signal) {
    case CAN_SIGNAL_POWER: state.power = payload.value; break;
    case CAN_SIGNAL_FAN_SPEED: state.fanSpeed = payload.value; break;
    case CAN_SIGNAL_TEMPERATURE: state.driverTemp = payload.value; break;
    case CAN_SIGNAL_PASSENGER_TEMPERATURE: state.passengerTemp = payload.value; break;
    case CAN_SIGNAL_MODE: state.windMode = payload.value; break;
    case CAN_SIGNAL_AC: state.ac = payload.value; break;
    case CAN_SIGNAL_AUTO: state.autoMode = payload.value; break;
    case CAN_SIGNAL_SCREEN_MODE: state.screenMode = payload.value; break;
    case CAN_SIGNAL_MEDIA: state.mediaReady = payload.value; break;
    case CAN_SIGNAL_VOLUME: state.volume = payload.value; break;
    case CAN_SIGNAL_MAP: state.mapReady = payload.value; break;
    case CAN_SIGNAL_MUTE: state.mute = payload.value; break;
    case CAN_SIGNAL_HOME: state.homeReady = payload.value; break;
    case CAN_SIGNAL_MEDIA_MODE: state.mediaMode = payload.value; break;
    case CAN_SIGNAL_MEDIA_INDEX:
      state.mediaIndex = payload.value;
      snprintf(state.media.title, sizeof(state.media.title), "Track %02u", state.mediaIndex);
      strncpy(state.media.artist, "Local Media", sizeof(state.media.artist) - 1);
      break;
  }
}

const char* huScreenToText(HuScreen screen) {
  switch (screen) {
    case HU_SCREEN_HOME: return "HOME";
    case HU_SCREEN_MAP: return "MAP";
    case HU_SCREEN_MEDIA: return "MEDIA";
    case HU_SCREEN_HVAC: return "HVAC";
    case HU_SCREEN_SETTING: return "SETTING";
    default: return "UNKNOWN";
  }
}

const char* huWindToText(uint8_t wind) {
  switch (wind) {
    case 0: return "FACE";
    case 1: return "FOOT";
    case 2: return "DEF";
    case 3: return "MIX";
    case 4: return "OFF";
    default: return "--";
  }
}

uint32_t dirtyForScreen(HuScreen screen) {
  switch (screen) {
    case HU_SCREEN_HOME: return DIRTY_HOME;
    case HU_SCREEN_MEDIA: return DIRTY_MEDIA;
    case HU_SCREEN_HVAC: return DIRTY_HVAC;
    case HU_SCREEN_MAP: return DIRTY_MAP;
    case HU_SCREEN_SETTING: return DIRTY_SETTING;
    default: return DIRTY_HOME;
  }
}

