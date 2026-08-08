#include "WifiManager.h"

#include <Arduino.h>
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
char connectedSsid[33] = {};

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
      WiFi.mode(WIFI_STA);
      setState(HEAD_UNIT_WIFI_IDLE);
      startScan();
      break;

    case WIFI_COMMAND_DISABLE:
      WiFi.disconnect(true, false);
      WiFi.mode(WIFI_OFF);
      connectedSsid[0] = '\0';
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
      WiFi.mode(WIFI_STA);
      WiFi.begin(command.ssid, command.password[0] == '\0' ? nullptr : command.password);
      copyText(connectedSsid, sizeof(connectedSsid), command.ssid);
      connectStartedMs = millis();
      setState(HEAD_UNIT_WIFI_CONNECTING);
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
  wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    String currentSsid = WiFi.SSID();
    copyText(connectedSsid, sizeof(connectedSsid), currentSsid.c_str());
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
      millis() - connectStartedMs >= kConnectTimeoutMs) {
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
  WiFi.mode(WIFI_OFF);
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
