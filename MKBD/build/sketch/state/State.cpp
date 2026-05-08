#line 1 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\state\\State.cpp"
#include "State.h"
#include "../GDS.h"

void initSystemState(SystemState& state) {
  state.screenMode = SCREEN_DATC;

  state.hvacMode = HVAC_OFF;
  state.fanSpeed = GDS_FAN_SPEED_MIN;
  state.setTemp = GDS_TEMP_DEFAULT;
  state.windMode = WIND_FACE;

  state.volume = GDS_VOLUME_DEFAULT;
  state.mediaReady = false;
  state.mapReady = false;
}

void toggleScreenMode(SystemState& state) {
  state.screenMode = (state.screenMode == SCREEN_DATC) ? SCREEN_INFO : SCREEN_DATC;
}

const char* screenModeToText(ScreenMode mode) {
  switch (mode) {
    case SCREEN_DATC: return "DATC";
    case SCREEN_INFO: return "INFO";
    default: return "UNKNOWN";
  }
}

const char* hvacModeToText(HvacMode mode) {
  switch (mode) {
    case HVAC_OFF:  return "OFF";
    case HVAC_AC:   return "AC";
    case HVAC_HEAT: return "HEAT";
    case HVAC_AUTO: return "AUTO";
    default: return "UNKNOWN";
  }
}

const char* windModeToText(WindMode mode) {
  switch (mode) {
    case WIND_FACE: return "FACE";
    case WIND_FOOT: return "FOOT";
    case WIND_DEF:  return "DEF";
    case WIND_MIX:  return "MIX";
    default: return "UNKNOWN";
  }
}
