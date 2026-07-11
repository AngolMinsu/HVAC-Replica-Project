#include "Datc.h"
#include "../GDS.h"

static uint8_t increaseTemp(int& temp) {
  int oldTemp = temp;
  if (temp < GDS_TEMP_MAX) temp++;
  return temp != oldTemp;
}

static uint8_t decreaseTemp(int& temp) {
  int oldTemp = temp;
  if (temp > GDS_TEMP_MIN) temp--;
  return temp != oldTemp;
}

uint8_t datcIncreaseDriverTemp(SystemState& state) {
  return increaseTemp(state.driverTemp);
}

uint8_t datcDecreaseDriverTemp(SystemState& state) {
  return decreaseTemp(state.driverTemp);
}

uint8_t datcIncreasePassengerTemp(SystemState& state) {
  return increaseTemp(state.passengerTemp);
}

uint8_t datcDecreasePassengerTemp(SystemState& state) {
  return decreaseTemp(state.passengerTemp);
}

uint8_t datcIncreaseFanSpeed(SystemState& state) {
  int oldFanSpeed = state.fanSpeed;
  if (state.fanSpeed < GDS_FAN_SPEED_MAX) state.fanSpeed++;
  return state.fanSpeed != oldFanSpeed;
}

uint8_t datcDecreaseFanSpeed(SystemState& state) {
  int oldFanSpeed = state.fanSpeed;
  if (state.fanSpeed > GDS_FAN_SPEED_MIN) state.fanSpeed--;
  return state.fanSpeed != oldFanSpeed;
}

uint8_t datcCycleWindMode(SystemState& state) {
  int nextWind = (int)state.windMode + 1;
  if (nextWind > WIND_OFF) nextWind = WIND_FACE;

  state.windMode = (WindMode)nextWind;

  return 1;
}

static void printTemp(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, int temp) {
  if (temp <= GDS_TEMP_MIN) {
    display.print("LO");
  } else if (temp >= GDS_TEMP_MAX) {
    display.print("HI");
  } else {
    display.print(temp);
    display.print("C");
  }
}

static void drawFanBar(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, int level) {
  int x = 0;
  int y = 52;
  int w = 10;
  int h = 8;

  for (int i = 0; i < GDS_FAN_SPEED_MAX; i++) {
    int bx = x + i * (w + 2);

    if (i < level) {
      display.drawBox(bx, y, w, h);
    } else {
      display.drawFrame(bx, y, w, h);
    }
  }
}

void datcDrawScreen(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, const SystemState& state) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tr);

  display.drawStr(0, 10, "DATC MODE");

  display.setCursor(70, 10);
  display.print("HVAC:");
  display.print(hvacModeToText(state.hvacMode));

  display.setCursor(0, 24);
  display.print("FAN:");
  display.print(state.fanSpeed);
  display.print("/");
  display.print(GDS_FAN_SPEED_MAX);

  display.setCursor(70, 24);
  display.print("AIR:");
  display.print(windModeToText(state.windMode));

  display.setCursor(0, 40);
  display.print("DRV:");
  printTemp(display, state.driverTemp);

  display.setCursor(70, 40);
  display.print("PSG:");
  printTemp(display, state.passengerTemp);

  drawFanBar(display, state.windMode == WIND_OFF ? 0 : state.fanSpeed);

  display.sendBuffer();
}
