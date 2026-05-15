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

static void drawVolumeBar(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, int volume) {
  int x = 0;
  int y = 42;
  int w = 100;
  int h = 8;

  display.drawFrame(x, y, w, h);

  int fillWidth = map(volume, GDS_VOLUME_MIN, GDS_VOLUME_MAX, 0, w - 2);
  display.drawBox(x + 1, y + 1, fillWidth, h - 2);
}

void infoDrawScreen(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, const SystemState& state) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tr);

  display.drawStr(0, 10, "INFO MODE");

  display.setCursor(0, 24);
  display.print("VOL:");
  display.print(state.mute ? "MUTE" : "");
  if (!state.mute) {
    display.print(state.volume);
    display.print("/");
    display.print(GDS_VOLUME_MAX);
  }

  display.setCursor(0, 36);
  display.print("RADIO:");
  display.print(state.radioMode ? "ON" : "OFF");
  display.print(" TUNE:");
  display.print(state.radioTune);

  drawVolumeBar(display, state.volume);

  display.setCursor(0, 62);
  display.print("MAP:");
  display.print(state.mapReady ? "READY" : "DEV");
  display.print(" NAV:");
  display.print(state.navReady ? "ON" : "OFF");

  display.sendBuffer();
}
