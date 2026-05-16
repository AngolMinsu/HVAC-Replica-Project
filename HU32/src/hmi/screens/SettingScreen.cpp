#include "SettingScreen.h"

void drawSettingScreen(TFT_eSPI& tft, const SystemState& state) {
  tft.fillRoundRect(44, 64, 392, 170, 14, 0x0824);
  tft.drawRoundRect(44, 64, 392, 170, 14, 0x294A);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("SYSTEM", 72, 92, 4);
  tft.setTextColor(0x9CF3);
  char line[64];
  snprintf(line, sizeof(line), "CAN %s  RX %s", state.canReady ? "READY" : "FAIL", state.canRxOk ? "OK" : "TIMEOUT");
  tft.drawString(line, 72, 144, 2);
  snprintf(line, sizeof(line), "SIG 0x%02X  VAL %u", state.lastSignal, state.lastValue);
  tft.drawString(line, 72, 174, 2);
  snprintf(line, sizeof(line), "HEAP %lu  FPS %lu", (unsigned long)state.freeHeap, (unsigned long)state.fps);
  tft.drawString(line, 72, 204, 2);
}

