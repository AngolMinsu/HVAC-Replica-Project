#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H

#include <stdint.h>

#include "../../GDS.h"

#define CAN_ID_CONTROL_REQUEST GDS_CAN_ID_CONTROL_REQUEST
#define CAN_ID_CONTROL_RESPONSE GDS_CAN_ID_CONTROL_RESPONSE
#define CAN_ID_HVAC_STATUS GDS_CAN_ID_HVAC_STATUS
#define CAN_DLC GDS_CAN_DLC

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

uint8_t canCalculateChecksum(const CanPayload& payload);
uint8_t canValidateChecksum(const CanPayload& payload);
CanPayload canPayloadFromBytes(const uint8_t bytes[CAN_DLC]);
void canPayloadToBytes(const CanPayload& payload, uint8_t bytes[CAN_DLC]);
const char* canSignalToText(uint8_t signal);

#endif
