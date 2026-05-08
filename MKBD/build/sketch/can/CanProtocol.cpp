#line 1 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\can\\CanProtocol.cpp"
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

CanPayload canMakePayload(uint8_t service, uint8_t result, uint8_t signal, uint8_t value, uint8_t option, uint8_t counter) {
  CanPayload payload;
  payload.service = service;
  payload.result = result;
  payload.signal = signal;
  payload.value = value;
  payload.option = option;
  payload.reserved = 0;
  payload.counter = counter;
  payload.checksum = canCalculateChecksum(payload);

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
