#line 1 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\can\\CanHandler.h"
#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include <Arduino.h>
#include "CanProtocol.h"
#include "../state/State.h"

uint8_t canApplyWriteRequest(SystemState& state, const CanPayload& request);
uint8_t canValidateWriteRequest(const CanPayload& request);
uint8_t canApplyReadRequest(const SystemState& state, const CanPayload& request, uint8_t& value);
uint8_t canValidateReadRequest(const CanPayload& request);
uint8_t canProcessControlRequest(SystemState& state, const CanPayload& request, CanPayload& response);
CanPayload canMakeStatusPayload(const SystemState& state, uint8_t signal, uint8_t counter);
uint8_t canSignalValueFromState(const SystemState& state, uint8_t signal, uint8_t& value);

#endif
