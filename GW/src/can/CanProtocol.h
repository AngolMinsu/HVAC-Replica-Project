#ifndef GW_CAN_PROTOCOL_H
#define GW_CAN_PROTOCOL_H

#include <Arduino.h>
#include "../../GDS.h"

struct CanPayload {
  uint8_t service;
  uint8_t result;
  uint8_t signal;
  uint8_t value;
  uint8_t option;
  uint8_t reserved;
  uint8_t counter;
  uint8_t checksum;
};

CanPayload canPayloadFromBytes(const uint8_t* data);
void canPayloadToBytes(const CanPayload& payload, uint8_t* data);
uint8_t canCalculateChecksum(const CanPayload& payload);
uint8_t canValidateChecksum(const CanPayload& payload);
uint8_t canIsKnownSignal(uint8_t signal);
const char* canSignalToText(uint8_t signal);

#endif
