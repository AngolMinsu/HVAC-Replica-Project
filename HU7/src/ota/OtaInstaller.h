#pragma once

#include <Arduino.h>

#include "OtaTypes.h"

namespace hu7::ota {

using OtaInstallProgressCallback = void (*)(uint8_t progress, void* context);

bool installFirmwareFromFile(const Manifest& manifest, const char* path,
                             OtaInstallProgressCallback progressCallback,
                             void* progressContext, char* error, size_t errorSize);

}  // namespace hu7::ota
