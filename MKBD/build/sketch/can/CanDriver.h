#line 1 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\can\\CanDriver.h"
#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include <Arduino.h>
#include "../GDS.h"

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
