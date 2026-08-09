#include "OtaManager.h"

#include <Preferences.h>
#include <WiFi.h>

#include "FirmwareIdentity.h"
#include "OtaHttpClient.h"
#include "OtaInstaller.h"
#include "../storage/StorageManager.h"

namespace hu7::ota {
namespace {

constexpr char kStagingDirectory[] = "/firmware/HU7";
constexpr char kStagingTemporaryPath[] = "/firmware/HU7/staged.part";
constexpr char kStagedFirmwarePath[] = "/firmware/HU7/staged.bin";

enum class CommandType : uint8_t { Check, Download, Install };

struct Command {
  CommandType type;
  UpdateTarget target;
};

QueueHandle_t commandQueue = nullptr;
SemaphoreHandle_t snapshotMutex = nullptr;
portMUX_TYPE commandMux = portMUX_INITIALIZER_UNLOCKED;
bool commandActive = false;
bool taskReady = false;
Snapshot current{};
Manifest availableManifest{};
Manifest stagedManifest{};
bool manifestReady = false;
bool stagedReady = false;
Preferences otaPreferences;
bool preferencesReady = false;
char installedVersion[24]{};

bool reserveCommand() {
  bool reserved = false;
  portENTER_CRITICAL(&commandMux);
  if (taskReady && !commandActive) {
    commandActive = true;
    reserved = true;
  }
  portEXIT_CRITICAL(&commandMux);
  return reserved;
}

void releaseCommand() {
  portENTER_CRITICAL(&commandMux);
  commandActive = false;
  portEXIT_CRITICAL(&commandMux);
}

void setTaskReady(bool ready) {
  portENTER_CRITICAL(&commandMux);
  taskReady = ready;
  portEXIT_CRITICAL(&commandMux);
}

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

void clearStagedMetadata(bool removeFirmware) {
  stagedReady = false;
  stagedManifest = {};
  if (preferencesReady) {
    otaPreferences.remove("stageVer");
    otaPreferences.remove("stageSize");
    otaPreferences.remove("stageSha");
  }
  if (removeFirmware && storageManagerIsReady()) {
    storageManagerRemoveFile(kStagedFirmwarePath);
    storageManagerRemoveFile(kStagingTemporaryPath);
  }
}

void persistStagedMetadata() {
  if (!preferencesReady || !stagedReady) return;
  otaPreferences.putString("stageVer", stagedManifest.version);
  otaPreferences.putULong("stageSize", stagedManifest.size);
  otaPreferences.putString("stageSha", stagedManifest.sha256);
}

void restoreStagedMetadata() {
  if (!preferencesReady || !storageManagerIsReady()) return;

  Manifest restored{};
  copyText(restored.target, sizeof(restored.target), kFirmwareTarget);
  otaPreferences.getString("stageVer", restored.version, sizeof(restored.version));
  otaPreferences.getString("stageSha", restored.sha256, sizeof(restored.sha256));
  restored.size = otaPreferences.getULong("stageSize", 0);

  const bool valid = restored.version[0] != '\0' && restored.sha256[0] != '\0' &&
                     restored.size > 0 &&
                     storageManagerFileSize(kStagedFirmwarePath) == restored.size &&
                     compareVersion(installedVersion, restored.version) < 0;
  if (!valid) {
    clearStagedMetadata(true);
    return;
  }

  stagedManifest = restored;
  stagedReady = true;
}

void fail(Snapshot& snapshot, const char* message, bool serverConnected,
          bool retryDownload = false, bool retryInstall = false) {
  snapshot.state = OtaState::Failed;
  snapshot.updateAvailable = retryDownload;
  snapshot.installReady = retryInstall;
  copyText(snapshot.serverStatus, sizeof(snapshot.serverStatus),
           serverConnected ? "Connected" : "Disconnected");
  copyText(snapshot.message, sizeof(snapshot.message), message);
  publish(snapshot);
}

bool stagedMatches(const Manifest& manifest) {
  return stagedReady && strcmp(stagedManifest.version, manifest.version) == 0 &&
         stagedManifest.size == manifest.size &&
         strcmp(stagedManifest.sha256, manifest.sha256) == 0;
}

void applyStagedSnapshot(Snapshot& snapshot) {
  snapshot.state = OtaState::ReadyToInstall;
  snapshot.downloadProgress = 100;
  snapshot.installProgress = 0;
  snapshot.updateAvailable = false;
  snapshot.installReady = true;
  snapshot.firmwareSize = stagedManifest.size;
  copyText(snapshot.latestVersion, sizeof(snapshot.latestVersion), stagedManifest.version);
  copyText(snapshot.packageTarget, sizeof(snapshot.packageTarget), stagedManifest.target);
  copyText(snapshot.targetStatus, sizeof(snapshot.targetStatus), "Staged");
  copyText(snapshot.message, sizeof(snapshot.message), "Download complete. Ready to install");
}

void checkLatest(UpdateTarget target) {
  manifestReady = false;
  Snapshot snapshot = readCurrent();
  snapshot.selectedTarget = target;
  snapshot.downloadProgress = stagedReady ? 100 : 0;
  snapshot.installProgress = 0;
  snapshot.firmwareSize = 0;
  snapshot.latestVersion[0] = '\0';
  snapshot.packageTarget[0] = '\0';
  snapshot.updateAvailable = false;
  snapshot.installReady = false;
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
    fail(snapshot, "Wi-Fi is disconnected", false, false, stagedReady);
    return;
  }

  snapshot.state = OtaState::Connecting;
  copyText(snapshot.serverStatus, sizeof(snapshot.serverStatus), "Connecting");
  copyText(snapshot.message, sizeof(snapshot.message), "Connecting to OTA server");
  publish(snapshot);

  OtaHttpClient client;
  char error[96]{};
  if (!client.health(error, sizeof(error))) {
    fail(snapshot, error, false, false, stagedReady);
    return;
  }

  snapshot.state = OtaState::CheckingVersion;
  copyText(snapshot.serverStatus, sizeof(snapshot.serverStatus), "Connected");
  copyText(snapshot.message, sizeof(snapshot.message), "Checking latest firmware");
  publish(snapshot);

  Manifest manifest{};
  if (!client.getLatest(target, manifest, error, sizeof(error))) {
    fail(snapshot, error, true, false, stagedReady);
    return;
  }
  if (strcmp(manifest.target, kFirmwareTarget) != 0) {
    fail(snapshot, "Package target mismatch", true, false, stagedReady);
    return;
  }

  copyText(snapshot.latestVersion, sizeof(snapshot.latestVersion), manifest.version);
  copyText(snapshot.packageTarget, sizeof(snapshot.packageTarget), manifest.target);
  snapshot.firmwareSize = manifest.size;
  if (compareVersion(installedVersion, manifest.version) >= 0) {
    snapshot.state = OtaState::Idle;
    snapshot.updateAvailable = false;
    snapshot.installReady = false;
    copyText(snapshot.message, sizeof(snapshot.message), "Firmware is up to date");
    publish(snapshot);
    return;
  }

  availableManifest = manifest;
  manifestReady = true;
  if (stagedMatches(manifest)) {
    applyStagedSnapshot(snapshot);
  } else {
    snapshot.state = OtaState::UpdateAvailable;
    snapshot.downloadProgress = 0;
    snapshot.installProgress = 0;
    snapshot.updateAvailable = true;
    snapshot.installReady = false;
    copyText(snapshot.message, sizeof(snapshot.message), "Update available. Press Download");
  }
  publish(snapshot);
}

void downloadProgress(uint8_t progress, void* context) {
  (void)context;
  Snapshot snapshot = readCurrent();
  snapshot.downloadProgress = progress;
  snapshot.state = progress < 100 ? OtaState::Downloading : OtaState::Verifying;
  copyText(snapshot.message, sizeof(snapshot.message),
           progress < 100 ? "Downloading firmware to SD" : "Verifying downloaded firmware");
  publish(snapshot);
}

void installProgress(uint8_t progress, void* context) {
  (void)context;
  Snapshot snapshot = readCurrent();
  snapshot.installProgress = progress;
  snapshot.state = OtaState::Installing;
  copyText(snapshot.message, sizeof(snapshot.message), "Installing firmware from SD");
  publish(snapshot);
}

void downloadAvailableUpdate(UpdateTarget target) {
  Snapshot snapshot = readCurrent();
  if (target != UpdateTarget::HU7 || !manifestReady ||
      strcmp(availableManifest.target, kFirmwareTarget) != 0) {
    fail(snapshot, "Check update again", true);
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    fail(snapshot, "Wi-Fi is disconnected", false, true);
    return;
  }
  if (!storageManagerIsReady() ||
      !storageManagerEnsureDirectory("/firmware") ||
      !storageManagerEnsureDirectory(kStagingDirectory)) {
    fail(snapshot, "SD staging directory unavailable", true, true);
    return;
  }

  snapshot.state = OtaState::Downloading;
  snapshot.downloadProgress = 0;
  snapshot.installProgress = 0;
  snapshot.updateAvailable = false;
  snapshot.installReady = false;
  copyText(snapshot.message, sizeof(snapshot.message), "Downloading firmware to SD");
  publish(snapshot);

  OtaHttpClient client;
  char error[96]{};
  if (!client.downloadToFile(availableManifest, kStagingTemporaryPath,
                             kStagedFirmwarePath, downloadProgress, nullptr,
                             error, sizeof(error))) {
    snapshot = readCurrent();
    fail(snapshot, error, WiFi.status() == WL_CONNECTED, true);
    return;
  }

  stagedManifest = availableManifest;
  stagedReady = true;
  persistStagedMetadata();
  snapshot = readCurrent();
  applyStagedSnapshot(snapshot);
  publish(snapshot);
}

void installStagedUpdate(UpdateTarget target) {
  Snapshot snapshot = readCurrent();
  if (target != UpdateTarget::HU7 || !stagedReady ||
      strcmp(stagedManifest.target, kFirmwareTarget) != 0) {
    fail(snapshot, "Download firmware again", false);
    return;
  }

  snapshot.state = OtaState::Installing;
  snapshot.installProgress = 0;
  snapshot.updateAvailable = false;
  snapshot.installReady = false;
  copyText(snapshot.targetStatus, sizeof(snapshot.targetStatus), "Installing");
  copyText(snapshot.message, sizeof(snapshot.message), "Installing firmware from SD");
  publish(snapshot);

  char error[96]{};
  if (!installFirmwareFromFile(stagedManifest, kStagedFirmwarePath,
                               installProgress, nullptr, error, sizeof(error))) {
    snapshot = readCurrent();
    fail(snapshot, error, strcmp(snapshot.serverStatus, "Connected") == 0,
         false, stagedReady);
    return;
  }

  clearStagedMetadata(true);
  snapshot = readCurrent();
  snapshot.state = OtaState::Rebooting;
  snapshot.installProgress = 100;
  snapshot.installReady = false;
  copyText(snapshot.targetStatus, sizeof(snapshot.targetStatus), "Installed");
  copyText(snapshot.message, sizeof(snapshot.message), "Install complete. Rebooting");
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
  commandQueue = xQueueCreate(1, sizeof(Command));
  setTaskReady(false);
  releaseCommand();
  copyText(installedVersion, sizeof(installedVersion), kFirmwareVersion);
  preferencesReady = otaPreferences.begin("hu7-ota", false);
  if (preferencesReady) {
    otaPreferences.putString("version", installedVersion);
  }
  restoreStagedMetadata();
  copyText(current.currentVersion, sizeof(current.currentVersion), installedVersion);
  copyText(current.serverStatus, sizeof(current.serverStatus), "Disconnected");
  copyText(current.targetStatus, sizeof(current.targetStatus), "Select target");
  copyText(current.message, sizeof(current.message),
           stagedReady ? "Select HU7 to install staged update" : "Select update target");
  current.downloadProgress = stagedReady ? 100 : 0;
  current.revision = 1;
}

void otaManagerSelectTarget(UpdateTarget target) {
  manifestReady = false;
  Snapshot snapshot = readCurrent();
  snapshot.selectedTarget = target;
  snapshot.state = OtaState::Idle;
  snapshot.updateAvailable = false;
  snapshot.installReady = false;
  snapshot.downloadProgress = 0;
  snapshot.installProgress = 0;
  snapshot.firmwareSize = 0;
  snapshot.latestVersion[0] = '\0';
  snapshot.packageTarget[0] = '\0';
  copyText(snapshot.currentVersion, sizeof(snapshot.currentVersion),
           target == UpdateTarget::HU7 ? installedVersion : "-");

  if (target == UpdateTarget::HU7 && stagedReady) {
    applyStagedSnapshot(snapshot);
  } else {
    copyText(snapshot.targetStatus, sizeof(snapshot.targetStatus),
             target == UpdateTarget::None ? "Select target" :
             target == UpdateTarget::HU7 ? "Ready" : "Not implemented");
    copyText(snapshot.message, sizeof(snapshot.message),
             target == UpdateTarget::None ? "Select update target" :
             target == UpdateTarget::HU7 ? "Press Check Update" : "CAN OTA not implemented");
  }
  publish(snapshot);
}

bool otaManagerRequestCheck() {
  if (commandQueue == nullptr) return false;
  const Snapshot snapshot = readCurrent();
  if (snapshot.selectedTarget != UpdateTarget::HU7 || !reserveCommand()) return false;
  const Command command{CommandType::Check, snapshot.selectedTarget};
  if (xQueueSend(commandQueue, &command, 0) == pdTRUE) return true;
  releaseCommand();
  return false;
}

bool otaManagerRequestUpdate() {
  if (commandQueue == nullptr) return false;
  const Snapshot snapshot = readCurrent();
  if (snapshot.selectedTarget != UpdateTarget::HU7) return false;

  CommandType type{};
  if (snapshot.installReady && stagedReady) {
    type = CommandType::Install;
  } else if (snapshot.updateAvailable && manifestReady) {
    type = CommandType::Download;
  } else {
    return false;
  }
  if (!reserveCommand()) return false;
  const Command command{type, snapshot.selectedTarget};
  if (xQueueSend(commandQueue, &command, 0) == pdTRUE) return true;
  releaseCommand();
  return false;
}

bool otaManagerReady() {
  bool ready = false;
  portENTER_CRITICAL(&commandMux);
  ready = taskReady;
  portEXIT_CRITICAL(&commandMux);
  return ready;
}

bool otaManagerGetSnapshot(Snapshot& snapshot) {
  if (snapshotMutex == nullptr) return false;
  snapshot = readCurrent();
  return true;
}

void otaManagerTask(void* parameter) {
  (void)parameter;
  setTaskReady(true);
  Command command{};
  for (;;) {
    if (xQueueReceive(commandQueue, &command, portMAX_DELAY) == pdTRUE) {
      if (command.type == CommandType::Check) {
        checkLatest(command.target);
      } else if (command.type == CommandType::Download) {
        downloadAvailableUpdate(command.target);
      } else if (command.type == CommandType::Install) {
        installStagedUpdate(command.target);
      }
      releaseCommand();
    }
  }
}

}  // namespace hu7::ota