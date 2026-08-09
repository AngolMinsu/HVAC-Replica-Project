#include "OtaInstaller.h"

#include <Update.h>
#include <mbedtls/sha256.h>

#include "../storage/StorageManager.h"

namespace hu7::ota {
namespace {

void setError(char* output, size_t outputSize, const char* text) {
  if (output == nullptr || outputSize == 0) return;
  snprintf(output, outputSize, "%s", text != nullptr ? text : "unknown error");
}

bool digestMatches(const uint8_t* digest, const char* expected) {
  char digestText[65]{};
  for (size_t index = 0; index < 32; ++index) {
    snprintf(digestText + index * 2, 3, "%02x", digest[index]);
  }
  return expected != nullptr && strcmp(digestText, expected) == 0;
}

}  // namespace

bool installFirmwareFromFile(const Manifest& manifest, const char* path,
                             OtaInstallProgressCallback progressCallback,
                             void* progressContext, char* error, size_t errorSize) {
  if (!storageManagerIsReady()) {
    setError(error, errorSize, "SD card is unavailable");
    return false;
  }
  if (storageManagerFileSize(path) != manifest.size) {
    setError(error, errorSize, "Staged firmware size mismatch");
    return false;
  }

  File firmware = storageManagerOpenFile(path, FILE_READ);
  if (!firmware || firmware.isDirectory()) {
    setError(error, errorSize, "Staged firmware open failed");
    firmware.close();
    return false;
  }
  if (firmware.read() != 0xE9 || !firmware.seek(0)) {
    setError(error, errorSize, "Invalid ESP32 firmware header");
    firmware.close();
    return false;
  }
  if (!Update.begin(manifest.size, U_FLASH)) {
    snprintf(error, errorSize, "OTA begin: %s", Update.errorString());
    firmware.close();
    return false;
  }

  mbedtls_sha256_context sha{};
  mbedtls_sha256_init(&sha);
  if (mbedtls_sha256_starts(&sha, 0) != 0) {
    setError(error, errorSize, "SHA-256 init failed");
    mbedtls_sha256_free(&sha);
    Update.abort();
    firmware.close();
    return false;
  }

  uint8_t buffer[4096];
  uint32_t written = 0;
  uint8_t lastProgress = 0;
  bool failed = false;
  while (written < manifest.size) {
    const size_t remaining = manifest.size - written;
    const size_t requested = min(sizeof(buffer), remaining);
    const size_t bytesRead = firmware.read(buffer, requested);
    if (bytesRead == 0) {
      setError(error, errorSize, "Staged firmware read failed");
      failed = true;
      break;
    }
    if (mbedtls_sha256_update(&sha, buffer, bytesRead) != 0) {
      setError(error, errorSize, "SHA-256 update failed");
      failed = true;
      break;
    }
    if (Update.write(buffer, bytesRead) != bytesRead) {
      snprintf(error, errorSize, "OTA write: %s", Update.errorString());
      failed = true;
      break;
    }
    written += static_cast<uint32_t>(bytesRead);
    const uint8_t progress = static_cast<uint8_t>((written * 100ULL) / manifest.size);
    if ((progress >= lastProgress + 5U || progress == 100U) && progressCallback != nullptr) {
      lastProgress = progress;
      progressCallback(progress, progressContext);
    }
    delay(2);
  }

  uint8_t digest[32]{};
  if (!failed && mbedtls_sha256_finish(&sha, digest) != 0) {
    setError(error, errorSize, "SHA-256 finish failed");
    failed = true;
  }
  mbedtls_sha256_free(&sha);
  firmware.close();

  if (!failed && !digestMatches(digest, manifest.sha256)) {
    setError(error, errorSize, "Staged firmware SHA-256 mismatch");
    failed = true;
  }
  if (failed) {
    Update.abort();
    return false;
  }
  if (!Update.end(false) || !Update.isFinished()) {
    snprintf(error, errorSize, "OTA finalize: %s", Update.errorString());
    Update.abort();
    return false;
  }
  return true;
}

}  // namespace hu7::ota
