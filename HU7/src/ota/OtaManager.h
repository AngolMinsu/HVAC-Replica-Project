#pragma once

#include "OtaTypes.h"

namespace hu7::ota {

void otaManagerBegin();
void otaManagerTask(void* parameter);
void otaManagerSelectTarget(UpdateTarget target);
bool otaManagerRequestCheck();
bool otaManagerRequestUpdate();
bool otaManagerGetSnapshot(Snapshot& snapshot);

}  // namespace hu7::ota
