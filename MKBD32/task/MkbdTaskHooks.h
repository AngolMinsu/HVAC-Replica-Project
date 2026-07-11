#ifndef MKBD_TASK_HOOKS_H
#define MKBD_TASK_HOOKS_H

#include <Arduino.h>
#include "../state/State.h"

extern SystemState state;
extern unsigned long lastDisplayTime;

uint8_t readButtonEvent();
uint8_t readEncoderEvent();
int updateFanMotor();
uint8_t drawCurrentScreen();
uint8_t printSystemStatus(const SystemState& s);
uint8_t processCanReceive();
uint8_t broadcastButtonMenuEvent(uint8_t button, const SystemState& inputState);
uint8_t broadcastEncoderSwitchEvent(uint8_t encoderEvent, const SystemState& inputState);
uint8_t broadcastChangedHvacStatus(const SystemState& before, const SystemState& after);

#endif
