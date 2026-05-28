#include "InputTask.h"

#include <Arduino.h>

#include "../../../../GDS.h"

void headUnitInputTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_TASK_INPUT_MS));
  }
}
