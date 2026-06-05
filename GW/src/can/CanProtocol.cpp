#include "CanProtocol.h"

CanPayload canPayloadFromBytes(const uint8_t* data) {
  CanPayload payload = {};
  payload.service = data[0];
  payload.result = data[1];
  payload.signal = data[2];
  payload.value = data[3];
  payload.option = data[4];
  payload.reserved = data[5];
  payload.counter = data[6];
  payload.checksum = data[7];
  return payload;
}

void canPayloadToBytes(const CanPayload& payload, uint8_t* data) {
  data[0] = payload.service;
  data[1] = payload.result;
  data[2] = payload.signal;
  data[3] = payload.value;
  data[4] = payload.option;
  data[5] = payload.reserved;
  data[6] = payload.counter;
  data[7] = payload.checksum;
}

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

uint8_t canIsKnownSignal(uint8_t signal) {
  return signal >= GDS_CAN_SIGNAL_POWER && signal <= GDS_CAN_SIGNAL_HU_OPEN_MEDIA;
}

const char* canSignalToText(uint8_t signal) {
  switch (signal) {
    case GDS_CAN_SIGNAL_POWER: return "POWER";
    case GDS_CAN_SIGNAL_FAN_SPEED: return "FAN";
    case GDS_CAN_SIGNAL_TEMPERATURE: return "DRV_TEMP";
    case GDS_CAN_SIGNAL_MODE: return "WIND";
    case GDS_CAN_SIGNAL_AC: return "AC";
    case GDS_CAN_SIGNAL_AUTO: return "AUTO";
    case GDS_CAN_SIGNAL_INTAKE: return "INTAKE";
    case GDS_CAN_SIGNAL_SCREEN_MODE: return "SCREEN";
    case GDS_CAN_SIGNAL_MEDIA: return "MEDIA";
    case GDS_CAN_SIGNAL_VOLUME: return "VOLUME";
    case GDS_CAN_SIGNAL_MAP: return "MAP";
    case GDS_CAN_SIGNAL_PASSENGER_TEMPERATURE: return "PSG_TEMP";
    case GDS_CAN_SIGNAL_MUTE: return "MUTE";
    case GDS_CAN_SIGNAL_HOME: return "HOME";
    case GDS_CAN_SIGNAL_MEDIA_MODE: return "MEDIA_MODE";
    case GDS_CAN_SIGNAL_MEDIA_INDEX: return "MEDIA_INDEX";
    case GDS_CAN_SIGNAL_DRIVER_ENCODER_SW: return "DRV_SW";
    case GDS_CAN_SIGNAL_PASSENGER_ENCODER_SW: return "PSG_SW";
    case GDS_CAN_SIGNAL_HU_FOCUS_PREV: return "HU_FOCUS_PREV";
    case GDS_CAN_SIGNAL_HU_FOCUS_NEXT: return "HU_FOCUS_NEXT";
    case GDS_CAN_SIGNAL_HU_OPEN_HOME: return "HU_OPEN_HOME";
    case GDS_CAN_SIGNAL_HU_OPEN_MAP: return "HU_OPEN_MAP";
    case GDS_CAN_SIGNAL_HU_OPEN_MEDIA: return "HU_OPEN_MEDIA";
    default: return "UNKNOWN";
  }
}
