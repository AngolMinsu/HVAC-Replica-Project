#include "HeadUnitApp.h"

#include <Arduino.h>

#include "../../GDS.h"
#include "../can/CanDriver.h"
#include "../driver/DisplayDriver.h"
#include "../task/task10ms/can/CanRxTask.h"
#include "../task/task10ms/can/CanTxTask.h"
#include "../task/task10ms/input/InputTask.h"
#include "../task/task20ms/ui/UiTask.h"
#include "HeadUnitHmi.h"

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

  headUnitHmiBegin();
  canDriverBegin();

  xTaskCreatePinnedToCore(headUnitCanRxTask, "CAN_RX", 4096, nullptr, 3, nullptr, 0);
  xTaskCreatePinnedToCore(headUnitCanTxTask, "CAN_TX", 4096, nullptr, 3, nullptr, 0);
  xTaskCreatePinnedToCore(headUnitInputTask, "INPUT", 4096, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(headUnitUiTask, "UI", 4096, nullptr, 2, nullptr, 1);

  Serial.println("HU7 ready");
}
