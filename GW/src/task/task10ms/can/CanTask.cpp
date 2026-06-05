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
    Serial.print("GW RX TWAI #");
    Serial.println(++twaiRxCount);
    canMonitorPrintFrame("GW RX TWAI", rxFrame);

    CanFrame txFrame = {};
    if (gatewayRouteFrame("TWAI->MCP", rxFrame, txFrame) && GDS_GW_FORWARD_ENABLED) {
      uint8_t sent = mcpCanDriverSend(txFrame);
      Serial.println(sent ? "GW TX MCP OK" : "GW TX MCP FAIL");
    }
  }
}

static void routeMcpToTwai() {
  CanFrame rxFrame = {};
  while (mcpCanDriverReceive(rxFrame)) {
    Serial.print("GW RX MCP #");
    Serial.println(++mcpRxCount);
    canMonitorPrintFrame("GW RX MCP", rxFrame);

    CanFrame txFrame = {};
    if (gatewayRouteFrame("MCP->TWAI", rxFrame, txFrame) && GDS_GW_FORWARD_ENABLED) {
      uint8_t sent = twaiCanDriverSend(txFrame);
      Serial.println(sent ? "GW TX TWAI OK" : "GW TX TWAI FAIL");
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
