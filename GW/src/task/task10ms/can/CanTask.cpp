#include "CanTask.h"

#include "../../../../GDS.h"
#include "../../../can/McpCanDriver.h"
#include "../../../can/TwaiCanDriver.h"
#include "../../../gateway/GatewayRouter.h"

static uint32_t huToMkbdCount = 0;
static uint32_t mkbdToHuCount = 0;

static void printRouteResult(const char* direction, uint32_t count, const CanFrame& frame, uint8_t sent) {
  Serial.print("GW ");
  Serial.print(direction);
  Serial.print(" #");
  Serial.print(count);
  Serial.print(" ID:0x");
  Serial.print(frame.id, HEX);
  Serial.print(" DLC:");
  Serial.print(frame.dlc);
  Serial.print(" ");
  Serial.println(sent ? "OK" : "FAIL");
}

static void routeHuToMkbd() {
  CanFrame rxFrame = {};
  while (twaiCanDriverReceive(rxFrame)) {
    CanFrame txFrame = {};
    if (!gatewayRouteFrame("HU->MKBD", rxFrame, txFrame)) {
      continue;
    }

    uint8_t sent = GDS_GW_FORWARD_ENABLED ? mcpCanDriverSend(txFrame) : 0;
    printRouteResult("HU->MKBD", ++huToMkbdCount, rxFrame, sent);
  }
}

static void routeMkbdToHu() {
  CanFrame rxFrame = {};
  while (mcpCanDriverReceive(rxFrame)) {
    CanFrame txFrame = {};
    if (!gatewayRouteFrame("MKBD->HU", rxFrame, txFrame)) {
      continue;
    }

    uint8_t sent = GDS_GW_FORWARD_ENABLED ? twaiCanDriverSend(txFrame) : 0;
    printRouteResult("MKBD->HU", ++mkbdToHuCount, rxFrame, sent);
  }
}

void gatewayCanTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    twaiCanDriverPollHealth();
    mcpCanDriverPollHealth();

    routeHuToMkbd();
    routeMkbdToHu();

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_TASK_CAN_MS));
  }
}