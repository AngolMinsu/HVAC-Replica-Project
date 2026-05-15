#include "Info.h"
#include "../GDS.h"

uint8_t infoIncreaseVolume(SystemState& state) {
  int oldVolume = state.volume;

  state.volume++;
  if (state.volume > GDS_VOLUME_MAX) state.volume = GDS_VOLUME_MAX;

  return state.volume != oldVolume;
}

uint8_t infoDecreaseVolume(SystemState& state) {
  int oldVolume = state.volume;

  state.volume--;
  if (state.volume < GDS_VOLUME_MIN) state.volume = GDS_VOLUME_MIN;

  return state.volume != oldVolume;
}

uint8_t infoToggleMute(SystemState& state) {
  state.mute = !state.mute;
  return 1;
}

uint8_t infoTuneUp(SystemState& state) {
  if (!state.radioMode) return 0;

  int oldTune = state.radioTune;
  if (state.radioTune < GDS_RADIO_TUNE_MAX) state.radioTune++;
  return state.radioTune != oldTune;
}

uint8_t infoTuneDown(SystemState& state) {
  if (!state.radioMode) return 0;

  int oldTune = state.radioTune;
  if (state.radioTune > GDS_RADIO_TUNE_MIN) state.radioTune--;
  return state.radioTune != oldTune;
}

uint8_t infoSelect(SystemState& state) {
  if (!state.radioMode) return 0;

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

uint8_t infoHandleRadio(SystemState& state) {
  state.radioMode = !state.radioMode;

  return 1;
}

void infoDrawScreen(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, const SystemState& state) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tr);

  display.drawStr(0, 10, "INFO MODE");

  display.setCursor(0, 24);
  display.print("VOL:");
  if (state.mute) {
    display.print("MUTE");
  } else {
    display.print(state.volume);
    display.print("/");
    display.print(GDS_VOLUME_MAX);
  }

  display.setCursor(70, 24);
  display.print("MUTE:");
  display.print(state.mute ? "ON" : "OFF");

  display.setCursor(0, 38);
  display.print("RADIO:");
  display.print(state.radioMode ? "ON" : "OFF");

  display.setCursor(70, 38);
  display.print("TUNE:");
  display.print(state.radioTune);

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
