#include "HvacScreen.h"

static void drawTempCard(TFT_eSPI& tft, int16_t x, const char* label, uint8_t temp) {
  tft.fillRoundRect(x, 74, 190, 118, 14, 0x0824);
  tft.drawRoundRect(x, 74, 190, 118, 14, 0x294A);
  tft.setTextColor(0x9CF3);
  tft.drawString(label, x + 18, 94, 2);
  tft.setTextColor(TFT_WHITE);
  char value[12];
  if (temp <= 18) {
    snprintf(value, sizeof(value), "LO");
  } else if (temp >= 30) {
    snprintf(value, sizeof(value), "HI");
  } else {
    snprintf(value, sizeof(value), "%uC", temp);
  }
  tft.drawString(value, x + 18, 124, 6);
}

void drawHvacScreen(TFT_eSPI& tft, const SystemState& state) {
  drawTempCard(tft, 36, "DRIVER", state.driverTemp);
  drawTempCard(tft, 254, "PASSENGER", state.passengerTemp);

  tft.setTextColor(TFT_WHITE);
  char fan[40];
  snprintf(fan, sizeof(fan), "FAN %u / 8", state.fanSpeed);
  tft.drawString(fan, 48, 218, 4);
  tft.drawRoundRect(168, 228, 250, 12, 6, 0x294A);
  tft.fillRoundRect(168, 228, map(state.fanSpeed, 0, 8, 0, 250), 12, 6, 0x5D9F);

  char mode[48];
  snprintf(mode, sizeof(mode), "%s  A/C %s  AUTO %s", huWindToText(state.windMode), state.ac ? "ON" : "OFF", state.autoMode ? "ON" : "OFF");
  tft.setTextColor(0x9CF3);
  tft.drawString(mode, 48, 250, 2);
}

