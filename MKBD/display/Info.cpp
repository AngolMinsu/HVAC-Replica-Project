#include "Info.h"
#include "../GDS.h"

uint8_t infoHandleButton2(SystemState& state) {
  // MEDIA 기능은 추후 개발 예정
  state.mediaReady = !state.mediaReady;

  return 1;
}

uint8_t infoHandleButton3(SystemState& state) {
  int oldVolume = state.volume;

  state.volume++;
  if (state.volume > GDS_VOLUME_MAX) state.volume = GDS_VOLUME_MAX;

  return state.volume != oldVolume;
}

uint8_t infoHandleButton4(SystemState& state) {
  int oldVolume = state.volume;

  state.volume--;
  if (state.volume < GDS_VOLUME_MIN) state.volume = GDS_VOLUME_MIN;

  return state.volume != oldVolume;
}

uint8_t infoHandleButton5(SystemState& state) {
  // MAP 기능은 추후 개발 예정
  state.mapReady = !state.mapReady;

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
  display.print("D2 MEDIA:");
  display.print(state.mediaReady ? "READY" : "DEV");

  display.setCursor(0, 36);
  display.print("VOL:");
  display.print(state.volume);
  display.print("/");
  display.print(GDS_VOLUME_MAX);

  drawVolumeBar(display, state.volume);

  display.setCursor(0, 62);
  display.print("D5 MAP:");
  display.print(state.mapReady ? "READY" : "DEV");

  display.sendBuffer();
}
