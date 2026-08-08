#include "OtaHttpClient.h"

#include <HTTPClient.h>
#include <NetworkClient.h>
#include <Update.h>
#include <WiFiClient.h>
#include <mbedtls/sha256.h>

#include "OtaConfig.h"

namespace hu7::ota {
namespace {

void setError(char* output, size_t outputSize, const char* text) {
  if (output == nullptr || outputSize == 0) return;
  snprintf(output, outputSize, "%s", text != nullptr ? text : "unknown error");
}

bool jsonString(const String& json, const char* key, char* output, size_t outputSize) {
  const String token = String('"') + key + '"';
  int position = json.indexOf(token);
  if (position < 0) return false;
  position = json.indexOf(':', position + token.length());
  if (position < 0) return false;
  position = json.indexOf('"', position + 1);
  if (position < 0) return false;

  size_t written = 0;
  bool escaped = false;
  for (++position; position < static_cast<int>(json.length()); ++position) {
    const char ch = json[position];
    if (!escaped && ch == '"') {
      if (written >= outputSize) return false;
      output[written] = '\0';
      return true;
    }
    if (!escaped && ch == '\\') {
      escaped = true;
      continue;
    }
    if (written + 1 >= outputSize) return false;
    output[written++] = ch;
    escaped = false;
  }
  return false;
}

bool jsonUnsigned(const String& json, const char* key, uint32_t& output) {
  const String token = String('"') + key + '"';
  int position = json.indexOf(token);
  if (position < 0) return false;
  position = json.indexOf(':', position + token.length());
  if (position < 0) return false;
  do {
    ++position;
  } while (position < static_cast<int>(json.length()) && isspace(json[position]));
  if (position >= static_cast<int>(json.length()) || !isdigit(json[position])) return false;
  uint64_t value = 0;
  while (position < static_cast<int>(json.length()) && isdigit(json[position])) {
    value = value * 10 + static_cast<uint8_t>(json[position] - '0');
    if (value > UINT32_MAX) return false;
    ++position;
  }
  output = static_cast<uint32_t>(value);
  return true;
}

}  // namespace

bool OtaHttpClient::get(const String& url, String& payload, int& statusCode,
                        char* error, size_t errorSize) {
  WiFiClient client;
  HTTPClient http;
  http.setConnectTimeout(kHttpTimeoutMs);
  http.setTimeout(kHttpTimeoutMs);
  if (!http.begin(client, url)) {
    setError(error, errorSize, "HTTP begin failed");
    return false;
  }

  statusCode = http.GET();
  if (statusCode > 0) payload = http.getString();
  http.end();
  if (statusCode <= 0) {
    setError(error, errorSize, HTTPClient::errorToString(statusCode).c_str());
    return false;
  }
  return true;
}

bool OtaHttpClient::health(char* error, size_t errorSize) {
  String payload;
  int statusCode = 0;
  if (!get(String(kServerBaseUrl) + "/api/health", payload, statusCode, error, errorSize)) {
    return false;
  }
  if (statusCode != HTTP_CODE_OK || payload.indexOf("\"status\":\"ok\"") < 0) {
    snprintf(error, errorSize, "Health HTTP %d", statusCode);
    return false;
  }
  return true;
}

bool OtaHttpClient::getLatest(UpdateTarget target, Manifest& manifest,
                              char* error, size_t errorSize) {
  String payload;
  int statusCode = 0;
  const String url = String(kServerBaseUrl) + "/api/firmware/latest?target=" + targetName(target);
  if (!get(url, payload, statusCode, error, errorSize)) return false;
  if (statusCode != HTTP_CODE_OK) {
    snprintf(error, errorSize, "Manifest HTTP %d", statusCode);
    return false;
  }
  if (!jsonString(payload, "target", manifest.target, sizeof(manifest.target)) ||
      !jsonString(payload, "version", manifest.version, sizeof(manifest.version)) ||
      !jsonString(payload, "file", manifest.file, sizeof(manifest.file)) ||
      !jsonString(payload, "url", manifest.url, sizeof(manifest.url)) ||
      !jsonString(payload, "sha256", manifest.sha256, sizeof(manifest.sha256)) ||
      !jsonUnsigned(payload, "size", manifest.size)) {
    setError(error, errorSize, "Invalid manifest JSON");
    return false;
  }
  return true;
}

bool OtaHttpClient::downloadAndInstall(const Manifest& manifest,
                                       OtaProgressCallback progressCallback,
                                       void* progressContext,
                                       char* error, size_t errorSize) {
  const String firmwareUrl = String(manifest.url).startsWith("http")
                                 ? String(manifest.url)
                                 : String(kServerBaseUrl) + manifest.url;
  WiFiClient networkClient;
  HTTPClient http;
  http.setConnectTimeout(kHttpTimeoutMs);
  http.setTimeout(kHttpTimeoutMs);
  if (!http.begin(networkClient, firmwareUrl)) {
    setError(error, errorSize, "Firmware HTTP begin failed");
    return false;
  }

  const int statusCode = http.GET();
  if (statusCode != HTTP_CODE_OK) {
    snprintf(error, errorSize, "Firmware HTTP %d", statusCode);
    http.end();
    return false;
  }
  const int contentLength = http.getSize();
  if (contentLength < 0 || static_cast<uint32_t>(contentLength) != manifest.size) {
    setError(error, errorSize, "Firmware size mismatch");
    http.end();
    return false;
  }
  if (!Update.begin(manifest.size, U_FLASH)) {
    snprintf(error, errorSize, "OTA begin: %s", Update.errorString());
    http.end();
    return false;
  }

  mbedtls_sha256_context sha{};
  mbedtls_sha256_init(&sha);
  if (mbedtls_sha256_starts(&sha, 0) != 0) {
    setError(error, errorSize, "SHA-256 init failed");
    mbedtls_sha256_free(&sha);
    Update.abort();
    http.end();
    return false;
  }

  NetworkClient& stream = http.getStream();
  uint8_t buffer[4096];
  uint32_t received = 0;
  uint32_t lastDataMs = millis();
  uint8_t lastProgress = 255;
  bool failed = false;
  while (received < manifest.size) {
    const int available = stream.available();
    if (available > 0) {
      const size_t remaining = manifest.size - received;
      const size_t chunkSize = min(static_cast<size_t>(available),
                                   min(sizeof(buffer), remaining));
      const int bytesRead = stream.readBytes(buffer, chunkSize);
      if (bytesRead <= 0) {
        setError(error, errorSize, "Firmware read failed");
        failed = true;
        break;
      }
      if (Update.write(buffer, static_cast<size_t>(bytesRead)) != static_cast<size_t>(bytesRead)) {
        snprintf(error, errorSize, "OTA write: %s", Update.errorString());
        failed = true;
        break;
      }
      if (mbedtls_sha256_update(&sha, buffer, static_cast<size_t>(bytesRead)) != 0) {
        setError(error, errorSize, "SHA-256 update failed");
        failed = true;
        break;
      }
      received += static_cast<uint32_t>(bytesRead);
      lastDataMs = millis();
      const uint8_t progress = static_cast<uint8_t>((received * 100ULL) / manifest.size);
      if (progress != lastProgress && progressCallback != nullptr) {
        lastProgress = progress;
        progressCallback(progress, progressContext);
      }
      continue;
    }
    if (!http.connected() || millis() - lastDataMs > 10000) {
      setError(error, errorSize, "Firmware download interrupted");
      failed = true;
      break;
    }
    delay(1);
  }

  uint8_t digest[32]{};
  if (!failed && mbedtls_sha256_finish(&sha, digest) != 0) {
    setError(error, errorSize, "SHA-256 finish failed");
    failed = true;
  }
  mbedtls_sha256_free(&sha);
  http.end();
  if (failed) {
    Update.abort();
    return false;
  }

  char digestText[65]{};
  for (size_t index = 0; index < sizeof(digest); ++index) {
    snprintf(digestText + index * 2, 3, "%02x", digest[index]);
  }
  if (strcmp(digestText, manifest.sha256) != 0) {
    setError(error, errorSize, "Firmware SHA-256 mismatch");
    Update.abort();
    return false;
  }
  if (!Update.end(false) || !Update.isFinished()) {
    snprintf(error, errorSize, "OTA finalize: %s", Update.errorString());
    return false;
  }
  return true;
}

}  // namespace hu7::ota
