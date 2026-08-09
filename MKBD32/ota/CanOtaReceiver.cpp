#include "CanOtaReceiver.h"

#include <Arduino.h>
#include <Update.h>

#include "../GDS.h"
#include "CanOtaProtocol.h"
#include "MkbdFirmwareIdentity.h"

namespace {

using namespace mkbd::canota;

struct ReceiverState {
  bool active = false;
  uint8_t session = 0;
  uint32_t imageSize = 0;
  uint32_t nextOffset = 0;
  uint32_t runningCrc = 0xFFFFFFFFUL;
  uint8_t framesInBlock = 0;
  uint32_t lastActivityMs = 0;
} receiver;

bool sendFrame(uint16_t id, const uint8_t data[8]) {
  CanFrame frame{};
  frame.id = id;
  frame.dlc = GDS_CAN_DLC;
  memcpy(frame.data, data, GDS_CAN_DLC);
  return canDriverSend(frame) != 0;
}

void sendFlow(FlowStatus status, uint8_t session, uint32_t nextOffset, uint8_t detail) {
  uint8_t data[8]{};
  data[0] = static_cast<uint8_t>(status);
  data[1] = GDS_CAN_OTA_TARGET_MKBD;
  data[2] = session;
  writeU24(&data[3], nextOffset);
  data[6] = detail;
  data[7] = crc8(data, 7);
  sendFrame(GDS_CAN_ID_OTA_FLOW, data);
}

void sendResult(uint8_t session, Result result, Error error) {
  uint8_t data[8]{};
  data[0] = static_cast<uint8_t>(InfoType::Result);
  data[1] = GDS_CAN_OTA_TARGET_MKBD;
  data[2] = session;
  data[3] = static_cast<uint8_t>(result);
  data[4] = static_cast<uint8_t>(error);
  data[7] = crc8(data, 7);
  sendFrame(GDS_CAN_ID_OTA_INFO, data);
}

void sendVersion(uint8_t session) {
  uint8_t data[8]{};
  data[0] = static_cast<uint8_t>(InfoType::Version);
  data[1] = GDS_CAN_OTA_TARGET_MKBD;
  data[2] = session;
  data[3] = MKBD_FIRMWARE_VERSION_MAJOR;
  data[4] = MKBD_FIRMWARE_VERSION_MINOR;
  data[5] = (uint8_t)MKBD_FIRMWARE_VERSION_PATCH;
  data[6] = (uint8_t)(MKBD_FIRMWARE_VERSION_PATCH >> 8);
  data[7] = crc8(data, 7);
  sendFrame(GDS_CAN_ID_OTA_INFO, data);
}

void resetReceiver(bool abortUpdate) {
  if (abortUpdate && receiver.active) Update.abort();
  receiver = {};
  receiver.runningCrc = 0xFFFFFFFFUL;
}

void failSession(Error error) {
  const uint8_t session = receiver.session;
  const uint32_t offset = receiver.nextOffset;
  if (receiver.active) Update.abort();
  receiver.active = false;
  sendFlow(FlowStatus::Error, session, offset, static_cast<uint8_t>(error));
  sendResult(session, Result::Failed, error);
  resetReceiver(false);
}

uint8_t progressPercent() {
  if (receiver.imageSize == 0) return 0;
  const uint32_t percent = (receiver.nextOffset * 100ULL) / receiver.imageSize;
  return percent > 100 ? 100 : (uint8_t)percent;
}

void handleControl(const CanFrame& frame) {
  if (!validProtectedFrame(frame.data, frame.dlc)) return;
  const ControlOpcode opcode = static_cast<ControlOpcode>(frame.data[0]);
  const uint8_t target = frame.data[1];
  const uint8_t session = frame.data[2];
  const uint32_t argument = readU32(&frame.data[3]);

  if (target != GDS_CAN_OTA_TARGET_MKBD) return;

  if (opcode == ControlOpcode::VersionQuery) {
    sendVersion(session);
    return;
  }

  if (opcode == ControlOpcode::Start) {
    if (argument == 0 || argument > 0xFFFFFFUL) {
      sendFlow(FlowStatus::Error, session, 0, static_cast<uint8_t>(Error::ImageTooLarge));
      return;
    }
    if (receiver.active && receiver.session == session && receiver.imageSize == argument) {
      receiver.lastActivityMs = millis();
      sendFlow(FlowStatus::Ready, session, receiver.nextOffset, progressPercent());
      return;
    }
    if (receiver.active) failSession(Error::InvalidState);
    resetReceiver(false);
    if (!Update.begin(argument, U_FLASH)) {
      sendFlow(FlowStatus::Error, session, 0, static_cast<uint8_t>(Error::UpdateBeginFailed));
      sendResult(session, Result::Failed, Error::UpdateBeginFailed);
      return;
    }
    receiver.active = true;
    receiver.session = session;
    receiver.imageSize = argument;
    receiver.lastActivityMs = millis();
    Serial.printf("CAN_OTA RX START session:%u size:%lu\n", session, (unsigned long)argument);
    sendFlow(FlowStatus::Ready, session, 0, 0);
    return;
  }

  if (opcode == ControlOpcode::Abort) {
    if (receiver.active && receiver.session == session) {
      Update.abort();
      receiver.active = false;
      sendResult(session, Result::Cancelled, Error::Cancelled);
      resetReceiver(false);
    }
    return;
  }

  if (!receiver.active) {
    sendFlow(FlowStatus::Error, session, 0, static_cast<uint8_t>(Error::InvalidState));
    return;
  }
  if (session != receiver.session) {
    sendFlow(FlowStatus::Error, session, receiver.nextOffset, static_cast<uint8_t>(Error::SessionMismatch));
    return;
  }
  receiver.lastActivityMs = millis();

  if (opcode == ControlOpcode::StatusQuery) {
    sendFlow(FlowStatus::BlockAck, session, receiver.nextOffset, progressPercent());
    return;
  }
  if (opcode != ControlOpcode::End) return;

  sendFlow(FlowStatus::Verifying, session, receiver.nextOffset, progressPercent());
  if (receiver.nextOffset != receiver.imageSize) {
    failSession(Error::OffsetMismatch);
    return;
  }
  const uint32_t calculatedCrc = receiver.runningCrc ^ 0xFFFFFFFFUL;
  if (calculatedCrc != argument) {
    Serial.printf("CAN_OTA CRC FAIL expected:%08lX actual:%08lX\n", (unsigned long)argument, (unsigned long)calculatedCrc);
    failSession(Error::Crc32Mismatch);
    return;
  }
  if (!Update.end(false)) {
    failSession(Error::InvalidImage);
    return;
  }

  receiver.active = false;
  sendFlow(FlowStatus::Verified, session, receiver.nextOffset, 100);
  sendResult(session, Result::Success, Error::None);
  Serial.println("CAN_OTA VERIFY:OK");
  delay(500);
  ESP.restart();
}

void handleData(const CanFrame& frame) {
  if (frame.dlc != GDS_CAN_DLC) return;
  const uint8_t session = frame.data[0];
  const uint32_t offset = readU24(&frame.data[1]);
  if (!receiver.active) {
    sendFlow(FlowStatus::Error, session, 0, static_cast<uint8_t>(Error::InvalidState));
    return;
  }
  if (session != receiver.session) {
    sendFlow(FlowStatus::Error, session, receiver.nextOffset, static_cast<uint8_t>(Error::SessionMismatch));
    return;
  }
  receiver.lastActivityMs = millis();
  if (offset < receiver.nextOffset) {
    sendFlow(FlowStatus::BlockAck, session, receiver.nextOffset, progressPercent());
    return;
  }
  if (offset > receiver.nextOffset) {
    sendFlow(FlowStatus::Error, session, receiver.nextOffset, static_cast<uint8_t>(Error::OffsetMismatch));
    return;
  }

  const uint32_t remaining = receiver.imageSize - receiver.nextOffset;
  const size_t writeLength = remaining < 4 ? remaining : 4;
  uint8_t bytes[4];
  memcpy(bytes, &frame.data[4], sizeof(bytes));
  if (Update.write(bytes, writeLength) != writeLength) {
    failSession(Error::FlashWriteFailed);
    return;
  }
  receiver.runningCrc = crc32Update(receiver.runningCrc, bytes, writeLength);
  receiver.nextOffset += writeLength;
  receiver.framesInBlock++;

  if (receiver.framesInBlock >= GDS_CAN_OTA_BLOCK_FRAMES || receiver.nextOffset == receiver.imageSize) {
    receiver.framesInBlock = 0;
    sendFlow(FlowStatus::BlockAck, session, receiver.nextOffset, progressPercent());
  }
}

}  // namespace

bool mkbdCanOtaHandleFrame(const CanFrame& frame) {
  if (frame.id == GDS_CAN_ID_OTA_CONTROL) {
    handleControl(frame);
    return true;
  }
  if (frame.id == GDS_CAN_ID_OTA_DATA) {
    handleData(frame);
    return true;
  }
  return frame.id == GDS_CAN_ID_OTA_FLOW || frame.id == GDS_CAN_ID_OTA_INFO;
}

void mkbdCanOtaTick() {
  if (receiver.active && millis() - receiver.lastActivityMs > GDS_CAN_OTA_SESSION_TIMEOUT_MS) {
    Serial.println("CAN_OTA SESSION TIMEOUT");
    failSession(Error::Timeout);
  }
}

bool mkbdCanOtaActive() {
  return receiver.active;
}