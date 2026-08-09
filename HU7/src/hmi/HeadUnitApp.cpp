#include "HeadUnitApp.h"

#include <Arduino.h>

#include "../../GDS.h"
#include "../can/CanDriver.h"
#include "../driver/DisplayDriver.h"
#include "../ota/OtaManager.h"
#include "../storage/StorageManager.h"
#include "../task/task10ms/can/CanRxTask.h"
#include "../task/task10ms/input/InputTask.h"
#include "../task/task20ms/ui/UiTask.h"
#include "HeadUnitHmi.h"
#include "WifiManager.h"

void headUnitAppBegin() {
  Serial.begin(GDS_SERIAL_BAUD);
  delay(300);
  Serial.println();
  Serial.println("HU7 Head Unit start");

  if (!displayDriverBegin()) {
    Serial.println("Display driver init failed");
    while (true) {
      delay(1000);
    }
  }

  storageManagerBegin();
  wifiManagerBegin();
  hu7::ota::otaManagerBegin();
  headUnitHmiBegin();
  canDriverBegin();

  xTaskCreatePinnedToCore(headUnitCanRxTask, "CAN_RX", 4096, nullptr, 3, nullptr, 0);
  const BaseType_t otaTaskResult =
      xTaskCreatePinnedToCore(hu7::ota::otaManagerTask, "OTA", 8192, nullptr, 1, nullptr, 0);
  if (otaTaskResult != pdPASS) {
    Serial.printf("OTA task creation failed, free heap=%u bytes\n", ESP.getFreeHeap());
  }
  xTaskCreatePinnedToCore(headUnitInputTask, "INPUT", 4096, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(headUnitUiTask, "UI", 4096, nullptr, 2, nullptr, 1);

  Serial.println("HU7 ready");
}
