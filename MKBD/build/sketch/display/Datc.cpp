#line 1 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\display\\Datc.cpp"
#include "Datc.h"
#include "../GDS.h"

uint8_t datcHandleButton2(SystemState& state) {
  int nextMode = (int)state.hvacMode + 1;
  if (nextMode > HVAC_AUTO) nextMode = HVAC_OFF;

  state.hvacMode = (HvacMode)nextMode;

  if (state.hvacMode == HVAC_OFF) {
    state.fanSpeed = 0;
  }

  if (state.hvacMode != HVAC_OFF && state.fanSpeed == 0) {
    state.fanSpeed = 1;
  }

  return 1;
}

uint8_t datcHandleButton3(SystemState& state) {
  state.fanSpeed++;
  if (state.fanSpeed > GDS_FAN_SPEED_MAX) state.fanSpeed = GDS_FAN_SPEED_MIN;

  return 1;
}

uint8_t datcHandleButton4(SystemState& state) {
  state.setTemp++;
  if (state.setTemp > GDS_TEMP_MAX) state.setTemp = GDS_TEMP_MIN;

  return 1;
}

uint8_t datcHandleButton5(SystemState& state) {
  int nextWind = (int)state.windMode + 1;
  if (nextWind > WIND_MIX) nextWind = WIND_FACE;

  state.windMode = (WindMode)nextWind;

  return 1;
}

static void drawFanBar(U8G2_SH1106_128X64_NONAME_F_HW_I2C& display, int level) {
  int x = 0;
  int y = 44;
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

  display.setCursor(0, 24);
  display.print("HVAC:");
  display.print(hvacModeToText(state.hvacMode));

  display.setCursor(72, 24);
  display.print("SET:");
  display.print(state.setTemp);
  display.print("C");

  display.setCursor(0, 38);
  display.print("FAN:");
  display.print(state.fanSpeed);
  display.print("/");
  display.print(GDS_FAN_SPEED_MAX);

  display.setCursor(72, 38);
  display.print("AIR:");
  display.print(windModeToText(state.windMode));

  drawFanBar(display, state.fanSpeed);

  display.setCursor(0, 64);
  if (state.hvacMode == HVAC_AUTO) {
    display.print("AUTO: SENSOR READY");
  } else if (state.hvacMode == HVAC_AC) {
    display.print("Cooling mode");
  } else if (state.hvacMode == HVAC_HEAT) {
    display.print("Heating mode");
  } else {
    display.print("System off");
  }

  display.sendBuffer();
}
