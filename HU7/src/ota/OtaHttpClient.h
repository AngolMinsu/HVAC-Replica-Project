#pragma once

#include <Arduino.h>

#include "OtaTypes.h"

namespace hu7::ota {

using OtaProgressCallback = void (*)(uint8_t progress, void* context);

class OtaHttpClient {
 public:
  bool health(char* error, size_t errorSize);
  bool getLatest(UpdateTarget target, Manifest& manifest, char* error, size_t errorSize);
  bool downloadToFile(const Manifest& manifest, const char* temporaryPath,
                      const char* finalPath, OtaProgressCallback progressCallback,
                      void* progressContext, char* error, size_t errorSize);

 private:
  bool get(const String& url, String& payload, int& statusCode, char* error, size_t errorSize);
};

}  // namespace hu7::ota
