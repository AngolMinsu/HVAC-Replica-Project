#ifndef GW_TWAI_CAN_DRIVER_H
#define GW_TWAI_CAN_DRIVER_H

#include "CanTypes.h"

uint8_t twaiCanDriverBegin();
uint8_t twaiCanDriverSend(const CanFrame& frame);
uint8_t twaiCanDriverReceive(CanFrame& frame);
uint8_t twaiCanDriverIsReady();
void twaiCanDriverPollHealth();

#endif
