#ifndef HU_CAN_DRIVER_H
#define HU_CAN_DRIVER_H

#include <Arduino.h>
#include "../../GDS.h"

struct CanFrame {
  uint16_t id;
  uint8_t dlc;
  uint8_t data[GDS_CAN_DLC];
};

uint8_t canDriverBegin(uint8_t csPin);
uint8_t canDriverSend(const CanFrame& frame);
uint8_t canDriverReceive(CanFrame& frame);
uint8_t canDriverIsReady();

#endif
