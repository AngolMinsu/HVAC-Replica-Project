#include "GatewayApp.h"

#include <Arduino.h>

#include "../../GDS.h"
#include "../can/CanDriver.h"
#include "../task/task10ms/can/CanTask.h"

void gatewayAppBegin() {
  Serial.begin(GDS_SERIAL_BAUD);
  delay(300);

  Serial.println();
  Serial.println("GW Gateway Node start");

  uint8_t canReady = canDriverBegin();
  Serial.print("GW CAN:");
  Serial.println(canReady ? "READY" : "FAIL");

  xTaskCreatePinnedToCore(
      gatewayCanTask,
      "GW_CAN_10MS",
      4096,
      NULL,
      2,
      NULL,
      1);
}

void gatewayAppLoop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
