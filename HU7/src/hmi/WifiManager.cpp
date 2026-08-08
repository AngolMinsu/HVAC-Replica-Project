#include "WifiManager.h"

#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace {

constexpr size_t kMaxNetworks = 16;
constexpr uint32_t kConnectTimeoutMs = 15000;

enum WifiCommandType : uint8_t {
  WIFI_COMMAND_ENABLE,
  WIFI_COMMAND_DISABLE,
  WIFI_COMMAND_SCAN,
  WIFI_COMMAND_CONNECT,
  WIFI_COMMAND_DISCONNECT,
};

struct WifiCommand {
  WifiCommandType type;
  char ssid[33];
  char password[64];
};

QueueHandle_t commandQueue = nullptr;
HeadUnitWifiState state = HEAD_UNIT_WIFI_OFF;
HeadUnitWifiNetwork networks[kMaxNetworks] = {};
size_t networkCount = 0;
uint32_t revision = 1;
uint32_t networkRevision = 1;
uint32_t connectStartedMs = 0;
uint32_t lastConnectionPollMs = 0;
char connectedSsid[33] = {};
char savedSsid[33] = {};
char savedPassword[64] = {};
char pendingSsid[33] = {};
char pendingPassword[64] = {};
Preferences wifiPreferences;
bool preferencesReady = false;

void copyText(char* destination, size_t destinationSize, const char* source) {
  if (destination == nullptr || destinationSize == 0) return;
  snprintf(destination, destinationSize, "%s", source == nullptr ? "" : source);
}

void setState(HeadUnitWifiState nextState) {
  if (state == nextState) return;
  state = nextState;
  revision++;
}

void clearNetworks() {
  networkCount = 0;
  memset(networks, 0, sizeof(networks));
  revision++;
  networkRevision++;
}

bool queueCommand(const WifiCommand& command) {
  return commandQueue != nullptr && xQueueSend(commandQueue, &command, 0) == pdTRUE;
}

void startScan();

void startConnection(const char* ssid, const char* password) {
  if (ssid == nullptr || ssid[0] == '\0') {
    setState(HEAD_UNIT_WIFI_IDLE);
    startScan();
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password != nullptr && password[0] != '\0' ? password : nullptr);
  copyText(pendingSsid, sizeof(pendingSsid), ssid);
  copyText(pendingPassword, sizeof(pendingPassword), password);
  copyText(connectedSsid, sizeof(connectedSsid), ssid);
  connectStartedMs = millis();
  setState(HEAD_UNIT_WIFI_CONNECTING);
}

void saveConnectedCredentials() {
  if (pendingSsid[0] == '\0') return;
  copyText(savedSsid, sizeof(savedSsid), pendingSsid);
  copyText(savedPassword, sizeof(savedPassword), pendingPassword);
  if (preferencesReady) {
    wifiPreferences.putBool("enabled", true);
    wifiPreferences.putString("ssid", savedSsid);
    wifiPreferences.putString("password", savedPassword);
  }
  pendingSsid[0] = '\0';
  pendingPassword[0] = '\0';
}

void startScan() {
  if (state == HEAD_UNIT_WIFI_OFF) return;

  WiFi.scanDelete();
  int16_t result = WiFi.scanNetworks(true, true);
  if (result == WIFI_SCAN_FAILED) {
    setState(HEAD_UNIT_WIFI_FAILED);
    return;
  }
  setState(HEAD_UNIT_WIFI_SCANNING);
}

void collectScanResults(int16_t resultCount) {
  clearNetworks();

  for (int16_t index = 0; index < resultCount; index++) {
    String ssid = WiFi.SSID(index);
    if (ssid.length() == 0) continue;

    int32_t rssi = WiFi.RSSI(index);
    size_t existingIndex = kMaxNetworks;
    for (size_t item = 0; item < networkCount; item++) {
      if (ssid.equals(networks[item].ssid)) {
        existingIndex = item;
        break;
      }
    }

    HeadUnitWifiNetwork candidate = {};
    copyText(candidate.ssid, sizeof(candidate.ssid), ssid.c_str());
    candidate.rssi = rssi;
    candidate.secured = WiFi.encryptionType(index) != WIFI_AUTH_OPEN;

    if (existingIndex < networkCount) {
      if (candidate.rssi > networks[existingIndex].rssi) {
        networks[existingIndex] = candidate;
      }
      continue;
    }

    if (networkCount < kMaxNetworks) {
      networks[networkCount++] = candidate;
    }
  }

  for (size_t left = 0; left < networkCount; left++) {
    for (size_t right = left + 1; right < networkCount; right++) {
      if (networks[right].rssi > networks[left].rssi) {
        HeadUnitWifiNetwork temporary = networks[left];
        networks[left] = networks[right];
        networks[right] = temporary;
      }
    }
  }

  networkRevision++;

  WiFi.scanDelete();
  setState(HEAD_UNIT_WIFI_IDLE);
}

void processCommand(const WifiCommand& command) {
  switch (command.type) {
    case WIFI_COMMAND_ENABLE:
      if (preferencesReady) wifiPreferences.putBool("enabled", true);
      WiFi.mode(WIFI_STA);
      if (savedSsid[0] != '\0') {
        startConnection(savedSsid, savedPassword);
      } else {
        setState(HEAD_UNIT_WIFI_IDLE);
        startScan();
      }
      break;

    case WIFI_COMMAND_DISABLE:
      if (preferencesReady) wifiPreferences.putBool("enabled", false);
      WiFi.disconnect(true, false);
      WiFi.mode(WIFI_OFF);
      connectedSsid[0] = '\0';
      pendingSsid[0] = '\0';
      pendingPassword[0] = '\0';
      clearNetworks();
      setState(HEAD_UNIT_WIFI_OFF);
      break;

    case WIFI_COMMAND_SCAN:
      startScan();
      break;

    case WIFI_COMMAND_CONNECT:
      if (command.ssid[0] == '\0') {
        setState(HEAD_UNIT_WIFI_FAILED);
        break;
      }
      startConnection(command.ssid, command.password);
      break;

    case WIFI_COMMAND_DISCONNECT:
      WiFi.disconnect(false, false);
      connectedSsid[0] = '\0';
      setState(HEAD_UNIT_WIFI_IDLE);
      break;
  }
}

void updateScan() {
  if (state != HEAD_UNIT_WIFI_SCANNING) return;

  int16_t scanResult = WiFi.scanComplete();
  if (scanResult == WIFI_SCAN_RUNNING) return;
  if (scanResult < 0) {
    setState(HEAD_UNIT_WIFI_FAILED);
    return;
  }
  collectScanResults(scanResult);
}

void updateConnection() {
  if (state != HEAD_UNIT_WIFI_CONNECTING && state != HEAD_UNIT_WIFI_CONNECTED) return;

  uint32_t now = millis();
  if (now - lastConnectionPollMs < 250) return;
  lastConnectionPollMs = now;

  wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    if (state != HEAD_UNIT_WIFI_CONNECTED) {
      String currentSsid = WiFi.SSID();
      copyText(connectedSsid, sizeof(connectedSsid), currentSsid.c_str());
      saveConnectedCredentials();
    }
    setState(HEAD_UNIT_WIFI_CONNECTED);
    return;
  }

  if (state == HEAD_UNIT_WIFI_CONNECTED) {
    connectedSsid[0] = '\0';
    setState(HEAD_UNIT_WIFI_FAILED);
    return;
  }

  if (state != HEAD_UNIT_WIFI_CONNECTING) return;
  if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL ||
      now - connectStartedMs >= kConnectTimeoutMs) {
    setState(HEAD_UNIT_WIFI_FAILED);
  }
}

}  // namespace

void wifiManagerBegin() {
  if (commandQueue == nullptr) {
    commandQueue = xQueueCreate(8, sizeof(WifiCommand));
  }
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  preferencesReady = wifiPreferences.begin("hu7-wifi", false);
  const bool enabled = preferencesReady && wifiPreferences.getBool("enabled", false);
  if (preferencesReady) {
    wifiPreferences.getString("ssid", savedSsid, sizeof(savedSsid));
    wifiPreferences.getString("password", savedPassword, sizeof(savedPassword));
  }

  if (enabled) {
    WiFi.mode(WIFI_STA);
    if (savedSsid[0] != '\0') {
      startConnection(savedSsid, savedPassword);
    } else {
      setState(HEAD_UNIT_WIFI_IDLE);
      startScan();
    }
  } else {
    WiFi.mode(WIFI_OFF);
  }
}

void wifiManagerTick() {
  WifiCommand command = {};
  while (commandQueue != nullptr && xQueueReceive(commandQueue, &command, 0) == pdTRUE) {
    processCommand(command);
  }
  updateScan();
  updateConnection();
}

bool wifiManagerRequestEnabled(bool enabled) {
  WifiCommand command = {};
  command.type = enabled ? WIFI_COMMAND_ENABLE : WIFI_COMMAND_DISABLE;
  return queueCommand(command);
}

bool wifiManagerRequestScan() {
  WifiCommand command = {};
  command.type = WIFI_COMMAND_SCAN;
  return queueCommand(command);
}

bool wifiManagerRequestConnect(const char* ssid, const char* password) {
  WifiCommand command = {};
  command.type = WIFI_COMMAND_CONNECT;
  copyText(command.ssid, sizeof(command.ssid), ssid);
  copyText(command.password, sizeof(command.password), password);
  return queueCommand(command);
}

bool wifiManagerRequestDisconnect() {
  WifiCommand command = {};
  command.type = WIFI_COMMAND_DISCONNECT;
  return queueCommand(command);
}

HeadUnitWifiState wifiManagerGetState() {
  return state;
}

uint32_t wifiManagerGetRevision() {
  return revision;
}

uint32_t wifiManagerGetNetworkRevision() {
  return networkRevision;
}

size_t wifiManagerGetNetworkCount() {
  return networkCount;
}

bool wifiManagerGetNetwork(size_t index, HeadUnitWifiNetwork* network) {
  if (network == nullptr || index >= networkCount) return false;
  *network = networks[index];
  return true;
}

void wifiManagerGetConnectedSsid(char* destination, size_t destinationSize) {
  copyText(destination, destinationSize, connectedSsid);
}
