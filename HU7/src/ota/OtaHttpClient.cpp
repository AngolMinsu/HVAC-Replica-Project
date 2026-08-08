#include "OtaHttpClient.h"

#include <HTTPClient.h>
#include <WiFiClient.h>

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

}  // namespace hu7::ota
