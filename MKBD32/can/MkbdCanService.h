#ifndef MKBD_CAN_SERVICE_H
#define MKBD_CAN_SERVICE_H

#include <Arduino.h>

#include "../state/State.h"

uint8_t mkbdCanServiceBegin();
uint8_t mkbdCanServiceProcessReceive(SystemState& state);
uint8_t mkbdCanServiceBroadcastButtonMenu(uint8_t button, const SystemState& state);
uint8_t mkbdCanServiceBroadcastEncoderSwitch(uint8_t encoderEvent, const SystemState& state);
uint8_t mkbdCanServiceBroadcastChanges(const SystemState& before, const SystemState& after);

#endif
