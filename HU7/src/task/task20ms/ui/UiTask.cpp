#include "UiTask.h"

#include <Arduino.h>

#include "../../../../GDS.h"
#include "../../../driver/DisplayDriver.h"
#include "../../../hmi/HeadUnitHmi.h"

void headUnitUiTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    headUnitHmiTick();
    displayDriverLoop();
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_UI_FRAME_MS));
  }
}
