#include "CanTask.h"

#include "../../../../GDS.h"
#include "../../../can/CanMonitor.h"
#include "../../../can/McpCanDriver.h"
#include "../../../can/TwaiCanDriver.h"
#include "../../../gateway/GatewayRouter.h"

static uint32_t twaiRxCount = 0;
static uint32_t mcpRxCount = 0;

static void routeTwaiToMcp() {
  CanFrame rxFrame = {};
  while (twaiCanDriverReceive(rxFrame)) {
    Serial.print("GW TWAI RX -> MCP TX RX#");
    Serial.println(++twaiRxCount);
    canMonitorPrintFrame("GW TWAI RX", rxFrame);

    CanFrame txFrame = {};
    if (gatewayRouteFrame("TWAI->MCP", rxFrame, txFrame) && GDS_GW_FORWARD_ENABLED) {
      uint8_t sent = mcpCanDriverSend(txFrame);
      Serial.println(sent ? "GW TWAI RX -> MCP TX OK" : "GW TWAI RX -> MCP TX FAIL");
    }
  }
}

static void routeMcpToTwai() {
  CanFrame rxFrame = {};
  while (mcpCanDriverReceive(rxFrame)) {
    Serial.print("GW MCP RX -> TWAI TX RX#");
    Serial.println(++mcpRxCount);
    canMonitorPrintFrame("GW MCP RX", rxFrame);

    CanFrame txFrame = {};
    if (gatewayRouteFrame("MCP->TWAI", rxFrame, txFrame) && GDS_GW_FORWARD_ENABLED) {
      uint8_t sent = twaiCanDriverSend(txFrame);
      Serial.println(sent ? "GW MCP RX -> TWAI TX OK" : "GW MCP RX -> TWAI TX FAIL");
    }
  }
}

void gatewayCanTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    twaiCanDriverPollHealth();
    mcpCanDriverPollHealth();
    gatewayRouterPrintStats();

    routeTwaiToMcp();
    routeMcpToTwai();

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_TASK_CAN_MS));
  }
}
