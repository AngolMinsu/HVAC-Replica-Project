#ifndef GW_MCP_CAN_DRIVER_H
#define GW_MCP_CAN_DRIVER_H

#include "CanTypes.h"

uint8_t mcpCanDriverBegin();
uint8_t mcpCanDriverSend(const CanFrame& frame);
uint8_t mcpCanDriverReceive(CanFrame& frame);
uint8_t mcpCanDriverIsReady();
void mcpCanDriverPollHealth();

#endif
