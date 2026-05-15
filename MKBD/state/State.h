#ifndef STATE_H
#define STATE_H

enum ScreenMode {
  SCREEN_DATC = 0,
  SCREEN_INFO
};

enum HvacMode {
  HVAC_OFF = 0,
  HVAC_AC,
  HVAC_HEAT,
  HVAC_AUTO
};

enum WindMode {
  WIND_FACE = 0,
  WIND_FOOT,
  WIND_DEF,
  WIND_MIX,
  WIND_OFF
};

struct SystemState {
  ScreenMode screenMode;

  HvacMode hvacMode;
  int fanSpeed;   // 0~8
  int driverTemp;     // 18~30
  int passengerTemp;  // 18~30
  WindMode windMode;

  int volume;     // 0~30
  bool mute;
  bool mediaReady;
  bool mapReady;
  bool navReady;
  bool mediaMode;
  int mediaIndex;
};

void initSystemState(SystemState& state);
void toggleScreenMode(SystemState& state);

const char* screenModeToText(ScreenMode mode);
const char* hvacModeToText(HvacMode mode);
const char* windModeToText(WindMode mode);

#endif
