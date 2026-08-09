#pragma once

#include "OtaTypes.h"

namespace hu7::ota {

void otaManagerBegin();
void otaManagerTask(void* parameter);
void otaManagerSelectTarget(UpdateTarget target);
bool otaManagerRequestCheck();
bool otaManagerRequestUpdate();
bool otaManagerReady();
bool otaManagerGetSnapshot(Snapshot& snapshot);
size_t otaManagerRefreshStoredPackages();
size_t otaManagerGetStoredPackageCount();
bool otaManagerGetStoredPackage(size_t index, StoredPackage& package);
bool otaManagerSelectStoredPackage(size_t index, char* error, size_t errorSize);
bool otaManagerDeleteStoredPackage(size_t index, char* error, size_t errorSize);

}  // namespace hu7::ota
