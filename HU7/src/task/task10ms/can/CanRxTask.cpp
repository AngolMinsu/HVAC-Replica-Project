#include "CanRxTask.h"

#include <Arduino.h>

#include "../../../../GDS.h"
#include "../../../can/CanDriver.h"
#include "../../../can/CanMonitor.h"

void headUnitCanRxTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    CanFrame frame;
    if (canDriverReceive(frame)) {
      canMonitorPrintFrame("RX", frame);
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_TASK_CAN_MS));
  }
}
