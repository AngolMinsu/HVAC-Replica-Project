#include "CanOtaTransport.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "../../GDS.h"
#include "../storage/StorageManager.h"
#include "CanOtaProtocol.h"

namespace hu7::ota {
namespace {

using namespace hu7::canota;

QueueHandle_t responseQueue = nullptr;
uint8_t sessionCounter = 0;

void setError(char* error, size_t errorSize, const char* message) {
  if (error != nullptr && errorSize > 0) snprintf(error, errorSize, "%s", message);
}

uint8_t nextSession() {
  if (++sessionCounter == 0) ++sessionCounter;
  return sessionCounter;
}

bool sendRaw(uint16_t id, const uint8_t data[8]) {
  CanFrame frame{};
  frame.id = id;
  frame.dlc = GDS_CAN_DLC;
  memcpy(frame.data, data, GDS_CAN_DLC);
  return canDriverSend(frame) != 0;
}

bool sendControl(ControlOpcode opcode, uint8_t session, uint32_t argument) {
  uint8_t data[8]{};
  data[0] = static_cast<uint8_t>(opcode);
  data[1] = GDS_CAN_OTA_TARGET_MKBD;
  data[2] = session;
  writeU32(&data[3], argument);
  data[7] = crc8(data, 7);
  return sendRaw(GDS_CAN_ID_OTA_CONTROL, data);
}

bool sendData(uint8_t session, uint32_t offset, const uint8_t bytes[4]) {
  uint8_t data[8]{};
  data[0] = session;
  writeU24(&data[1], offset);
  memcpy(&data[4], bytes, 4);
  return sendRaw(GDS_CAN_ID_OTA_DATA, data);
}

bool receiveFrame(uint8_t session, CanFrame& frame, uint32_t timeoutMs) {
  if (responseQueue == nullptr) return false;
  const uint32_t started = millis();
  while (millis() - started < timeoutMs) {
    const uint32_t remaining = timeoutMs - (millis() - started);
    if (xQueueReceive(responseQueue, &frame, pdMS_TO_TICKS(remaining > 20 ? 20 : remaining)) != pdTRUE) continue;
    if (frame.dlc == GDS_CAN_DLC && frame.data[1] == GDS_CAN_OTA_TARGET_MKBD && frame.data[2] == session) return true;
  }
  return false;
}

bool waitFlow(uint8_t session, FlowStatus wanted, uint32_t timeoutMs,
              uint32_t& nextOffset, uint8_t& detail, Error& remoteError) {
  const uint32_t started = millis();
  CanFrame frame{};
  while (millis() - started < timeoutMs) {
    const uint32_t remaining = timeoutMs - (millis() - started);
    if (!receiveFrame(session, frame, remaining)) return false;
    if (frame.id == GDS_CAN_ID_OTA_FLOW) {
      const FlowStatus status = static_cast<FlowStatus>(frame.data[0]);
      nextOffset = readU24(&frame.data[3]);
      detail = frame.data[6];
      if (status == FlowStatus::Error) {
        remoteError = static_cast<Error>(detail);
        return false;
      }
      if (status == wanted) return true;
    } else if (frame.id == GDS_CAN_ID_OTA_INFO && frame.data[0] == static_cast<uint8_t>(InfoType::Result)) {
      if (frame.data[3] == static_cast<uint8_t>(Result::Success) && wanted == FlowStatus::Verified) return true;
      if (frame.data[3] != static_cast<uint8_t>(Result::Success)) {
        remoteError = static_cast<Error>(frame.data[4]);
        return false;
      }
    }
  }
  return false;
}

bool computeFileCrc32(File& file, uint32_t expectedSize, uint32_t& crc, char* error, size_t errorSize) {
  if (!file.seek(0)) {
    setError(error, errorSize, "MKBD firmware seek failed");
    return false;
  }
  uint8_t buffer[1024];
  uint32_t remaining = expectedSize;
  uint32_t running = 0xFFFFFFFFUL;
  while (remaining > 0) {
    const size_t wanted = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
    const size_t read = file.read(buffer, wanted);
    if (read != wanted) {
      setError(error, errorSize, "MKBD firmware read failed");
      return false;
    }
    running = crc32Update(running, buffer, read);
    remaining -= read;
  }
  crc = running ^ 0xFFFFFFFFUL;
  return file.seek(0);
}

bool startRemote(uint8_t session, uint32_t size, char* error, size_t errorSize) {
  for (uint8_t attempt = 0; attempt < GDS_CAN_OTA_MAX_RETRIES; ++attempt) {
    xQueueReset(responseQueue);
    if (!sendControl(ControlOpcode::Start, session, size)) continue;
    uint32_t next = 0;
    uint8_t detail = 0;
    Error remoteError = Error::None;
    if (waitFlow(session, FlowStatus::Ready, GDS_CAN_OTA_RESPONSE_TIMEOUT_MS, next, detail, remoteError) && next == 0) return true;
    if (remoteError != Error::None) {
      snprintf(error, errorSize, "MKBD START error 0x%02X", static_cast<unsigned>(remoteError));
      return false;
    }
  }
  setError(error, errorSize, "MKBD START timeout");
  return false;
}

bool transferBlocks(File& file, uint8_t session, uint32_t size,
                    CanOtaProgressCallback callback, void* context,
                    char* error, size_t errorSize) {
  uint32_t blockStart = 0;
  while (blockStart < size) {
    const uint32_t blockEnd = min<uint32_t>(blockStart + GDS_CAN_OTA_BLOCK_FRAMES * 4UL, size);
    bool acknowledged = false;
    for (uint8_t attempt = 0; attempt < GDS_CAN_OTA_MAX_RETRIES && !acknowledged; ++attempt) {
      xQueueReset(responseQueue);
      if (!file.seek(blockStart)) {
        setError(error, errorSize, "MKBD firmware seek failed");
        return false;
      }
      bool sendFailed = false;
      uint8_t burstFrames = 0;
      for (uint32_t offset = blockStart; offset < blockEnd; offset += 4) {
        uint8_t bytes[4]{};
        const size_t wanted = min<uint32_t>(4, size - offset);
        if (file.read(bytes, wanted) != wanted || !sendData(session, offset, bytes)) {
          sendFailed = true;
          break;
        }
        if (++burstFrames >= 8) {
          burstFrames = 0;
          vTaskDelay(1);
        }
      }
      if (sendFailed) continue;

      uint32_t next = 0;
      uint8_t detail = 0;
      Error remoteError = Error::None;
      if (waitFlow(session, FlowStatus::BlockAck, GDS_CAN_OTA_RESPONSE_TIMEOUT_MS,
                   next, detail, remoteError) && next == blockEnd) {
        acknowledged = true;
      } else if (remoteError != Error::None && remoteError != Error::OffsetMismatch) {
        snprintf(error, errorSize, "MKBD transfer error 0x%02X", static_cast<unsigned>(remoteError));
        return false;
      }
    }
    if (!acknowledged) {
      setError(error, errorSize, "MKBD block ACK timeout");
      return false;
    }
    blockStart = blockEnd;
    if (callback != nullptr) callback((uint8_t)((blockStart * 90ULL) / size), context);
  }
  return true;
}

bool finishRemote(uint8_t session, uint32_t crc, char* error, size_t errorSize) {
  for (uint8_t attempt = 0; attempt < GDS_CAN_OTA_MAX_RETRIES; ++attempt) {
    xQueueReset(responseQueue);
    if (!sendControl(ControlOpcode::End, session, crc)) continue;
    uint32_t next = 0;
    uint8_t detail = 0;
    Error remoteError = Error::None;
    if (waitFlow(session, FlowStatus::Verified, 1500, next, detail, remoteError)) return true;
    if (remoteError != Error::None) {
      snprintf(error, errorSize, "MKBD verify error 0x%02X", static_cast<unsigned>(remoteError));
      return false;
    }
  }
  setError(error, errorSize, "MKBD verify timeout");
  return false;
}

}  // namespace

void canOtaTransportBegin() {
  if (responseQueue == nullptr) responseQueue = xQueueCreate(16, sizeof(CanFrame));
}

bool canOtaTransportHandleFrame(const CanFrame& frame) {
  if (frame.id != GDS_CAN_ID_OTA_FLOW && frame.id != GDS_CAN_ID_OTA_INFO) return false;
  if (!validProtectedFrame(frame.data, frame.dlc)) {
    Serial.println("CAN_OTA RX DROP:CRC8");
    return true;
  }
  if (frame.data[1] != GDS_CAN_OTA_TARGET_MKBD || responseQueue == nullptr) return true;
  xQueueSend(responseQueue, &frame, 0);
  return true;
}

bool canOtaQueryMkbdVersion(char* version, size_t versionSize, char* error, size_t errorSize) {
  if (responseQueue == nullptr || !canDriverIsReady()) {
    setError(error, errorSize, "MKBD CAN unavailable");
    return false;
  }
  const uint8_t session = nextSession();
  for (uint8_t attempt = 0; attempt < GDS_CAN_OTA_MAX_RETRIES; ++attempt) {
    xQueueReset(responseQueue);
    if (!sendControl(ControlOpcode::VersionQuery, session, 0)) continue;
    CanFrame frame{};
    const uint32_t started = millis();
    while (millis() - started < GDS_CAN_OTA_RESPONSE_TIMEOUT_MS) {
      if (!receiveFrame(session, frame, GDS_CAN_OTA_RESPONSE_TIMEOUT_MS - (millis() - started))) break;
      if (frame.id == GDS_CAN_ID_OTA_INFO && frame.data[0] == static_cast<uint8_t>(InfoType::Version)) {
        const uint16_t patch = (uint16_t)frame.data[5] | ((uint16_t)frame.data[6] << 8);
        snprintf(version, versionSize, "%u.%u.%u", frame.data[3], frame.data[4], patch);
        return true;
      }
    }
  }
  setError(error, errorSize, "MKBD version timeout");
  return false;
}

bool canOtaInstallMkbd(const char* firmwarePath, uint32_t firmwareSize,
                       const char* expectedVersion, CanOtaProgressCallback progressCallback,
                       void* progressContext, char* error, size_t errorSize) {
  if (responseQueue == nullptr || !canDriverIsReady()) {
    setError(error, errorSize, "MKBD CAN unavailable");
    return false;
  }
  File file = storageManagerOpenFile(firmwarePath, FILE_READ);
  if (!file || file.size() != firmwareSize || firmwareSize == 0 || firmwareSize > 0xFFFFFFUL) {
    if (file) file.close();
    setError(error, errorSize, "MKBD staged firmware invalid");
    return false;
  }

  uint32_t firmwareCrc = 0;
  if (!computeFileCrc32(file, firmwareSize, firmwareCrc, error, errorSize)) {
    file.close();
    return false;
  }
  const uint8_t session = nextSession();
  Serial.printf("CAN_OTA START target:MKBD session:%u size:%lu\n", session, (unsigned long)firmwareSize);
  if (progressCallback != nullptr) progressCallback(0, progressContext);

  if (!startRemote(session, firmwareSize, error, errorSize) ||
      !transferBlocks(file, session, firmwareSize, progressCallback, progressContext, error, errorSize)) {
    sendControl(ControlOpcode::Abort, session, static_cast<uint32_t>(Error::Timeout));
    file.close();
    return false;
  }
  file.close();
  if (progressCallback != nullptr) progressCallback(95, progressContext);
  if (!finishRemote(session, firmwareCrc, error, errorSize)) return false;

  if (progressCallback != nullptr) progressCallback(98, progressContext);
  char runningVersion[24]{};
  char queryError[64]{};
  for (uint8_t attempt = 0; attempt < 10; ++attempt) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (canOtaQueryMkbdVersion(runningVersion, sizeof(runningVersion), queryError, sizeof(queryError))) {
      if (expectedVersion == nullptr || expectedVersion[0] == '\0' || strcmp(runningVersion, expectedVersion) == 0) {
        if (progressCallback != nullptr) progressCallback(100, progressContext);
        Serial.printf("CAN_OTA POST_CHECK expected:%s running:%s\n", expectedVersion, runningVersion);
        return true;
      }
      snprintf(error, errorSize, "MKBD version mismatch: %s", runningVersion);
      return false;
    }
  }
  setError(error, errorSize, "MKBD reboot version timeout");
  return false;
}

}  // namespace hu7::ota