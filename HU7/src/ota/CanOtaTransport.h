#pragma once

#include <Arduino.h>

#include "../can/CanDriver.h"

namespace hu7::ota {

using CanOtaProgressCallback = void (*)(uint8_t progress, void* context);

void canOtaTransportBegin();
bool canOtaTransportHandleFrame(const CanFrame& frame);
bool canOtaQueryMkbdVersion(char* version, size_t versionSize, char* error, size_t errorSize);
bool canOtaInstallMkbd(const char* firmwarePath, uint32_t firmwareSize,
                       const char* expectedVersion, CanOtaProgressCallback progressCallback,
                       void* progressContext, char* error, size_t errorSize);

}  // namespace hu7::ota