#ifndef GW_ROUTER_H
#define GW_ROUTER_H

#include "../can/CanTypes.h"

uint8_t gatewayRouteFrame(const char* sourceName, const CanFrame& rxFrame, CanFrame& txFrame);
void gatewayRouterPrintStats();

#endif
