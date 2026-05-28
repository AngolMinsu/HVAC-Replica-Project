#include "CanTxTask.h"

#include <Arduino.h>

#include "../../../../GDS.h"

void headUnitCanTxTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_TASK_CAN_MS));
  }
}
