#include "CanProtocol.h"

uint8_t canCalculateChecksum(const CanPayload& payload) {
  return payload.service ^
         payload.result ^
         payload.signal ^
         payload.value ^
         payload.option ^
         payload.reserved ^
         payload.counter;
}

uint8_t canValidateChecksum(const CanPayload& payload) {
  return canCalculateChecksum(payload) == payload.checksum;
}

CanPayload canPayloadFromBytes(const uint8_t bytes[CAN_DLC]) {
  CanPayload payload;
  payload.service = bytes[0];
  payload.result = bytes[1];
  payload.signal = bytes[2];
  payload.value = bytes[3];
  payload.option = bytes[4];
  payload.reserved = bytes[5];
  payload.counter = bytes[6];
  payload.checksum = bytes[7];
  return payload;
}

void canPayloadToBytes(const CanPayload& payload, uint8_t bytes[CAN_DLC]) {
  bytes[0] = payload.service;
  bytes[1] = payload.result;
  bytes[2] = payload.signal;
  bytes[3] = payload.value;
  bytes[4] = payload.option;
  bytes[5] = payload.reserved;
  bytes[6] = payload.counter;
  bytes[7] = payload.checksum;
}

const char* canSignalToText(uint8_t signal) {
  switch (signal) {
    case GDS_CAN_SIGNAL_POWER: return "Power";
    case GDS_CAN_SIGNAL_FAN_SPEED: return "Fan";
    case GDS_CAN_SIGNAL_TEMPERATURE: return "DrvTemp";
    case GDS_CAN_SIGNAL_MODE: return "Wind";
    case GDS_CAN_SIGNAL_AC: return "A/C";
    case GDS_CAN_SIGNAL_AUTO: return "Auto";
    case GDS_CAN_SIGNAL_INTAKE: return "Intake";
    case GDS_CAN_SIGNAL_SCREEN_MODE: return "Screen";
    case GDS_CAN_SIGNAL_MEDIA: return "Media";
    case GDS_CAN_SIGNAL_VOLUME: return "Volume";
    case GDS_CAN_SIGNAL_MAP: return "Map";
    case GDS_CAN_SIGNAL_PASSENGER_TEMPERATURE: return "PsgTemp";
    case GDS_CAN_SIGNAL_MUTE: return "Mute";
    case GDS_CAN_SIGNAL_HOME: return "Home";
    case GDS_CAN_SIGNAL_MEDIA_MODE: return "MediaMode";
    case GDS_CAN_SIGNAL_MEDIA_INDEX: return "MediaIndex";
    case GDS_CAN_SIGNAL_DRIVER_ENCODER_SW: return "DrvEncSw";
    case GDS_CAN_SIGNAL_PASSENGER_ENCODER_SW: return "PsgEncSw";
    case GDS_CAN_SIGNAL_HU_FOCUS_PREV: return "HuFocusPrev";
    case GDS_CAN_SIGNAL_HU_FOCUS_NEXT: return "HuFocusNext";
    case GDS_CAN_SIGNAL_HU_OPEN_HOME: return "HuOpenHome";
    case GDS_CAN_SIGNAL_HU_OPEN_MAP: return "HuOpenMap";
    case GDS_CAN_SIGNAL_HU_OPEN_MEDIA: return "HuOpenMedia";
    default: return "Unknown";
  }
}
