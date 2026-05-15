#include "Info.h"
#include "../GDS.h"

uint8_t infoIncreaseVolume(SystemState& state) {
  int oldVolume = state.volume;
  uint8_t changed = infoClearMute(state);

  state.volume++;
  if (state.volume > GDS_VOLUME_MAX) state.volume = GDS_VOLUME_MAX;

  return changed || state.volume != oldVolume;
}

uint8_t infoDecreaseVolume(SystemState& state) {
  int oldVolume = state.volume;
  uint8_t changed = infoClearMute(state);

  state.volume--;
  if (state.volume < GDS_VOLUME_MIN) state.volume = GDS_VOLUME_MIN;

  return changed || state.volume != oldVolume;
}

uint8_t infoToggleMute(SystemState& state) {
  state.mute = !state.mute;
  return 1;
}

uint8_t infoClearMute(SystemState& state) {
  if (!state.mute) return 0;

  state.mute = false;
  return 1;
}

uint8_t infoMediaIndexUp(SystemState& state) {
  if (!state.mediaMode) return 0;

  int oldIndex = state.mediaIndex;
  if (state.mediaIndex < GDS_MEDIA_INDEX_MAX) state.mediaIndex++;
  return state.mediaIndex != oldIndex;
}

uint8_t infoMediaIndexDown(SystemState& state) {
  if (!state.mediaMode) return 0;

  int oldIndex = state.mediaIndex;
  if (state.mediaIndex > GDS_MEDIA_INDEX_MIN) state.mediaIndex--;
  return state.mediaIndex != oldIndex;
}

uint8_t infoSelect(SystemState& state) {
  if (!state.mediaMode) return 0;

  state.mediaReady = !state.mediaReady;
  return 1;
}

uint8_t infoHandleMap(SystemState& state) {
  state.mapReady = !state.mapReady;

  return 1;
}

uint8_t infoHandleNav(SystemState& state) {
  state.navReady = !state.navReady;

  return 1;
}

uint8_t infoHandleMedia(SystemState& state) {
  state.mediaMode = !state.mediaMode;

  return 1;
}

void infoDrawScreen(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, const SystemState& state) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tr);

  display.drawStr(0, 10, "INFO MODE");

  display.setCursor(0, 24);
  if (state.mute) {
    display.print("MUTE");
  } else {
    display.print("VOL:");
    display.print(state.volume);
    display.print("/");
    display.print(GDS_VOLUME_MAX);
  }

  display.setCursor(0, 38);
  display.print("MEDIA:");
  display.print(state.mediaMode ? "ON" : "OFF");

  display.setCursor(70, 38);
  display.print("IDX:");
  display.print(state.mediaIndex);

  display.setCursor(0, 52);
  display.print("MAP:");
  display.print(state.mapReady ? "ON" : "OFF");

  display.setCursor(70, 52);
  display.print("NAV:");
  display.print(state.navReady ? "ON" : "OFF");

  display.setCursor(0, 64);
  display.print("MEDIA:");
  display.print(state.mediaReady ? "READY" : "DEV");

  display.sendBuffer();
}
