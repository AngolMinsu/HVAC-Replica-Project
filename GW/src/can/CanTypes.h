#ifndef GW_CAN_TYPES_H
#define GW_CAN_TYPES_H

#include <Arduino.h>
#include "../../GDS.h"

struct CanFrame {
  uint16_t id;
  uint8_t dlc;
  uint8_t data[GDS_CAN_DLC];
};

#endif
