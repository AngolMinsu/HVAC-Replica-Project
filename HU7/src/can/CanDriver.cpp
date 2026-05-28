#include "CanDriver.h"

#include <Arduino.h>

static uint8_t canReady = 0;

uint8_t canDriverBegin() {
  canReady = 0;
  Serial.print("CAN:");
  Serial.println(GDS_CAN_ENABLED ? "TODO" : "SKIP");
  return canReady;
}

uint8_t canDriverSend(const CanFrame& frame) {
  (void)frame;
  return 0;
}

uint8_t canDriverReceive(CanFrame& frame) {
  (void)frame;
  return 0;
}

uint8_t canDriverIsReady() {
  return canReady;
}
