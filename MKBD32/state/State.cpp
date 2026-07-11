#include "State.h"
#include "../GDS.h"

void initSystemState(SystemState& state) {
  state.screenMode = SCREEN_DATC;

  state.hvacMode = HVAC_OFF;
  state.fanSpeed = GDS_FAN_SPEED_MIN;
  state.driverTemp = GDS_TEMP_DEFAULT;
  state.passengerTemp = GDS_TEMP_DEFAULT;
  state.windMode = WIND_FACE;

  state.volume = GDS_VOLUME_DEFAULT;
  state.mute = false;
  state.mediaReady = false;
  state.mapReady = false;
  state.homeReady = false;
  state.mediaMode = false;
  state.mediaIndex = GDS_MEDIA_INDEX_DEFAULT;
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
    case WIND_OFF:  return "OFF";
    default: return "UNKNOWN";
  }
}
