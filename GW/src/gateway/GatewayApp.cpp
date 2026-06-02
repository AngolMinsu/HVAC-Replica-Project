#include "GatewayApp.h"

#include <Arduino.h>

#include "../../GDS.h"
#include "../can/McpCanDriver.h"
#include "../can/TwaiCanDriver.h"
#include "../task/task10ms/can/CanTask.h"

void gatewayAppBegin() {
  Serial.begin(GDS_SERIAL_BAUD);
  delay(300);

  Serial.println();
  Serial.println("GW Gateway Node start");

  uint8_t twaiReady = twaiCanDriverBegin();
  Serial.print("GW TWAI:");
  Serial.println(twaiReady ? "READY" : "FAIL");

  uint8_t mcpReady = mcpCanDriverBegin();
  Serial.print("GW MCP:");
  Serial.println(mcpReady ? "READY" : "FAIL");

  xTaskCreatePinnedToCore(
      gatewayCanTask,
      "GW_CAN_10MS",
      4096,
      NULL,
      3,
      NULL,
      1);
}

void gatewayAppLoop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
