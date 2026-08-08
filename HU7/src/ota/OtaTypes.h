#pragma once

#include <Arduino.h>

namespace hu7::ota {

enum class UpdateTarget : uint8_t {
  None = 0,
  HU7,
  MKBD,
  GW,
  BMS,
};

enum class OtaState : uint8_t {
  Idle = 0,
  Connecting,
  CheckingVersion,
  UpdateAvailable,
  Downloading,
  Verifying,
  Rebooting,
  Failed,
};

struct Manifest {
  char target[12]{};
  char version[24]{};
  char file[96]{};
  char url[192]{};
  char sha256[65]{};
  uint32_t size = 0;
};

struct Snapshot {
  uint32_t revision = 0;
  OtaState state = OtaState::Idle;
  UpdateTarget selectedTarget = UpdateTarget::None;
  char currentVersion[24]{};
  char latestVersion[24]{};
  char serverStatus[24]{};
  char targetStatus[32]{};
  char packageTarget[12]{};
  char message[96]{};
  uint32_t firmwareSize = 0;
  uint8_t progress = 0;
  bool updateAvailable = false;
};

const char* targetName(UpdateTarget target);

}  // namespace hu7::ota
