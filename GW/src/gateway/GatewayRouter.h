#ifndef GW_ROUTER_H
#define GW_ROUTER_H

#include "../can/CanDriver.h"

uint8_t gatewayRouteFrame(const CanFrame& rxFrame, CanFrame& txFrame);

#endif
