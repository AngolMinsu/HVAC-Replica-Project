#pragma once

#include <Arduino.h>

namespace hu7::canota {

enum class ControlOpcode : uint8_t { VersionQuery = 0x01, Start = 0x02, End = 0x03, Abort = 0x04, StatusQuery = 0x05 };
enum class FlowStatus : uint8_t { Ready = 0x10, BlockAck = 0x11, Verifying = 0x12, Verified = 0x13, Error = 0x1F };
enum class InfoType : uint8_t { Version = 0x20, Result = 0x21 };
enum class Result : uint8_t { Success = 0x00, Failed = 0x01, Cancelled = 0x02 };
enum class Error : uint8_t {
  None = 0x00, InvalidTarget = 0x01, InvalidState = 0x02, SessionMismatch = 0x03,
  OffsetMismatch = 0x04, ImageTooLarge = 0x05, UpdateBeginFailed = 0x06,
  FlashWriteFailed = 0x07, Crc32Mismatch = 0x08, InvalidImage = 0x09,
  Timeout = 0x0A, Cancelled = 0x0B, CanTxFailed = 0x0C, VersionTimeout = 0x0D,
  SdError = 0x0E, Sha256Mismatch = 0x0F, ServerError = 0x10
};

inline uint8_t crc8(const uint8_t* data, size_t length) {
  uint8_t crc = 0xFF;
  for (size_t i = 0; i < length; ++i) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; ++bit) crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x1D) : (uint8_t)(crc << 1);
  }
  return (uint8_t)(crc ^ 0xFF);
}

inline bool validProtectedFrame(const uint8_t* data, uint8_t dlc) { return dlc == 8 && crc8(data, 7) == data[7]; }
inline uint32_t readU24(const uint8_t* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16); }
inline void writeU24(uint8_t* p, uint32_t v) { p[0] = v; p[1] = v >> 8; p[2] = v >> 16; }
inline void writeU32(uint8_t* p, uint32_t v) { writeU24(p, v); p[3] = v >> 24; }
inline uint32_t crc32Update(uint32_t crc, const uint8_t* data, size_t length) {
  for (size_t i = 0; i < length; ++i) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320UL : 0);
  }
  return crc;
}

}  // namespace hu7::canota