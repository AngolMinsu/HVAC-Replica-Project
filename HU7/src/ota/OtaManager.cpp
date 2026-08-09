#include "OtaManager.h"

#include <Preferences.h>
#include <WiFi.h>

#include "FirmwareIdentity.h"
#include "OtaHttpClient.h"

namespace hu7::ota {
namespace {

enum class CommandType : uint8_t { Check, Update };

struct Command {
  CommandType type;
  UpdateTarget target;
};

QueueHandle_t commandQueue = nullptr;
SemaphoreHandle_t snapshotMutex = nullptr;
Snapshot current{};
Manifest availableManifest{};
bool manifestReady = false;
Preferences otaPreferences;
bool preferencesReady = false;
char installedVersion[24]{};

void copyText(char* output, size_t outputSize, const char* value) {
  snprintf(output, outputSize, "%s", value != nullptr ? value : "-");
}

void publish(const Snapshot& next) {
  if (snapshotMutex == nullptr) return;
  xSemaphoreTake(snapshotMutex, portMAX_DELAY);
  const uint32_t revision = current.revision + 1;
  current = next;
  current.revision = revision;
  xSemaphoreGive(snapshotMutex);
}

Snapshot readCurrent() {
  Snapshot result{};
  if (snapshotMutex == nullptr) return result;
  xSemaphoreTake(snapshotMutex, portMAX_DELAY);
  result = current;
  xSemaphoreGive(snapshotMutex);
  return result;
}

int compareVersion(const char* left, const char* right) {
  const char* a = left;
  const char* b = right;
  while (*a != '\0' || *b != '\0') {
    char* aEnd = nullptr;
    char* bEnd = nullptr;
    const unsigned long aPart = strtoul(a, &aEnd, 10);
    const unsigned long bPart = strtoul(b, &bEnd, 10);
    if (aPart < bPart) return -1;
    if (aPart > bPart) return 1;
    a = (*aEnd == '.') ? aEnd + 1 : aEnd;
    b = (*bEnd == '.') ? bEnd + 1 : bEnd;
    if (aEnd == a && bEnd == b) break;
  }
  return 0;
}

void fail(Snapshot& snapshot, const char* message, bool serverConnected) {
  snapshot.state = OtaState::Failed;
  snapshot.updateAvailable = false;
  copyText(snapshot.serverStatus, sizeof(snapshot.serverStatus),
           serverConnected ? "Connected" : "Disconnected");
  copyText(snapshot.message, sizeof(snapshot.message), message);
  publish(snapshot);
}

void checkLatest(UpdateTarget target) {
  manifestReady = false;
  Snapshot snapshot = readCurrent();
  snapshot.selectedTarget = target;
  snapshot.progress = 0;
  snapshot.firmwareSize = 0;
  snapshot.latestVersion[0] = '\0';
  snapshot.packageTarget[0] = '\0';
  snapshot.updateAvailable = false;
  copyText(snapshot.currentVersion, sizeof(snapshot.currentVersion),
           target == UpdateTarget::HU7 ? installedVersion : "-");

  if (target == UpdateTarget::None) {
    fail(snapshot, "Select update target", false);
    return;
  }
  if (target != UpdateTarget::HU7) {
    copyText(snapshot.targetStatus, sizeof(snapshot.targetStatus), "Not implemented");
    fail(snapshot, "CAN OTA not implemented", false);
    return;
  }
  copyText(snapshot.targetStatus, sizeof(snapshot.targetStatus), "Ready");
  if (WiFi.status() != WL_CONNECTED) {
    fail(snapshot, "Wi-Fi is disconnected", false);
    return;
  }

  snapshot.state = OtaState::Connecting;
  copyText(snapshot.serverStatus, sizeof(snapshot.serverStatus), "Connecting");
  copyText(snapshot.message, sizeof(snapshot.message), "Connecting to OTA server");
  publish(snapshot);

  OtaHttpClient client;
  char error[96]{};
  if (!client.health(error, sizeof(error))) {
    fail(snapshot, error, false);
    return;
  }

  snapshot.state = OtaState::CheckingVersion;
  copyText(snapshot.serverStatus, sizeof(snapshot.serverStatus), "Connected");
  copyText(snapshot.message, sizeof(snapshot.message), "Checking latest firmware");
  publish(snapshot);

  Manifest manifest{};
  if (!client.getLatest(target, manifest, error, sizeof(error))) {
    fail(snapshot, error, true);
    return;
  }
  if (strcmp(manifest.target, kFirmwareTarget) != 0) {
    fail(snapshot, "Package target mismatch", true);
    return;
  }

  copyText(snapshot.latestVersion, sizeof(snapshot.latestVersion), manifest.version);
  copyText(snapshot.packageTarget, sizeof(snapshot.packageTarget), manifest.target);
  snapshot.firmwareSize = manifest.size;
  if (compareVersion(installedVersion, manifest.version) < 0) {
    availableManifest = manifest;
    manifestReady = true;
    snapshot.state = OtaState::UpdateAvailable;
    snapshot.updateAvailable = true;
    copyText(snapshot.message, sizeof(snapshot.message), "Update available");
  } else {
    snapshot.state = OtaState::Idle;
    snapshot.updateAvailable = false;
    copyText(snapshot.message, sizeof(snapshot.message), "Firmware is up to date");
  }
  publish(snapshot);
}

void updateProgress(uint8_t progress, void* context) {
  (void)context;
  Snapshot snapshot = readCurrent();
  snapshot.progress = progress;
  snapshot.state = progress < 100 ? OtaState::Downloading : OtaState::Verifying;
  copyText(snapshot.message, sizeof(snapshot.message),
           progress < 100 ? "Downloading firmware" : "Verifying firmware");
  publish(snapshot);
}

void installAvailableUpdate(UpdateTarget target) {
  Snapshot snapshot = readCurrent();
  if (target != UpdateTarget::HU7 || !manifestReady ||
      strcmp(availableManifest.target, kFirmwareTarget) != 0) {
    fail(snapshot, "Check update again", true);
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    fail(snapshot, "Wi-Fi is disconnected", false);
    return;
  }

  snapshot.state = OtaState::Downloading;
  snapshot.progress = 0;
  snapshot.updateAvailable = false;
  copyText(snapshot.message, sizeof(snapshot.message), "Downloading firmware");
  publish(snapshot);

  OtaHttpClient client;
  char error[96]{};
  if (!client.downloadAndInstall(availableManifest, updateProgress, nullptr,
                                 error, sizeof(error))) {
    snapshot = readCurrent();
    fail(snapshot, error, WiFi.status() == WL_CONNECTED);
    return;
  }

  manifestReady = false;
  copyText(installedVersion, sizeof(installedVersion), availableManifest.version);
  if (preferencesReady) otaPreferences.putString("version", installedVersion);
  snapshot = readCurrent();
  snapshot.state = OtaState::Rebooting;
  snapshot.progress = 100;
  copyText(snapshot.message, sizeof(snapshot.message), "Update complete. Rebooting");
  publish(snapshot);
  delay(1500);
  ESP.restart();
}

}  // namespace

const char* targetName(UpdateTarget target) {
  switch (target) {
    case UpdateTarget::HU7: return "HU7";
    case UpdateTarget::MKBD: return "MKBD";
    case UpdateTarget::GW: return "GW";
    case UpdateTarget::BMS: return "BMS";
    default: return "";
  }
}

void otaManagerBegin() {
  snapshotMutex = xSemaphoreCreateMutex();
  commandQueue = xQueueCreate(2, sizeof(Command));
  copyText(installedVersion, sizeof(installedVersion), kFirmwareVersion);
  preferencesReady = otaPreferences.begin("hu7-ota", false);
  if (preferencesReady) {
    otaPreferences.putString("version", installedVersion);
  }
  copyText(current.currentVersion, sizeof(current.currentVersion), installedVersion);
  copyText(current.serverStatus, sizeof(current.serverStatus), "Disconnected");
  copyText(current.targetStatus, sizeof(current.targetStatus), "Select target");
  copyText(current.message, sizeof(current.message), "Select update target");
  current.revision = 1;
}

void otaManagerSelectTarget(UpdateTarget target) {
  manifestReady = false;
  Snapshot snapshot = readCurrent();
  snapshot.selectedTarget = target;
  snapshot.state = OtaState::Idle;
  snapshot.updateAvailable = false;
  snapshot.progress = 0;
  snapshot.firmwareSize = 0;
  snapshot.latestVersion[0] = '\0';
  snapshot.packageTarget[0] = '\0';
  copyText(snapshot.currentVersion, sizeof(snapshot.currentVersion),
           target == UpdateTarget::HU7 ? installedVersion : "-");
  copyText(snapshot.targetStatus, sizeof(snapshot.targetStatus),
           target == UpdateTarget::None ? "Select target" :
           target == UpdateTarget::HU7 ? "Ready" : "Not implemented");
  copyText(snapshot.message, sizeof(snapshot.message),
           target == UpdateTarget::None ? "Select update target" :
           target == UpdateTarget::HU7 ? "Press Check Update" : "CAN OTA not implemented");
  publish(snapshot);
}

bool otaManagerRequestCheck() {
  if (commandQueue == nullptr) return false;
  const Snapshot snapshot = readCurrent();
  const Command command{CommandType::Check, snapshot.selectedTarget};
  return xQueueSend(commandQueue, &command, 0) == pdTRUE;
}

bool otaManagerRequestUpdate() {
  if (commandQueue == nullptr) return false;
  const Snapshot snapshot = readCurrent();
  if (!snapshot.updateAvailable || snapshot.selectedTarget != UpdateTarget::HU7) return false;
  const Command command{CommandType::Update, snapshot.selectedTarget};
  return xQueueSend(commandQueue, &command, 0) == pdTRUE;
}

bool otaManagerGetSnapshot(Snapshot& snapshot) {
  if (snapshotMutex == nullptr) return false;
  snapshot = readCurrent();
  return true;
}

void otaManagerTask(void* parameter) {
  (void)parameter;
  Command command{};
  for (;;) {
    if (xQueueReceive(commandQueue, &command, portMAX_DELAY) == pdTRUE) {
      if (command.type == CommandType::Check) {
        checkLatest(command.target);
      } else if (command.type == CommandType::Update) {
        installAvailableUpdate(command.target);
      }
    }
  }
}

}  // namespace hu7::ota
