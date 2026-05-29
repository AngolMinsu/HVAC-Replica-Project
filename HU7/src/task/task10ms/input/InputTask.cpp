#include "InputTask.h"

#include <Arduino.h>

#include "../../../../GDS.h"
#include "../../../hmi/HeadUnitHmi.h"

static void handleDebugInput(char key) {
  switch (key) {
    case 'h':
    case 'H':
      headUnitHmiOpenHome();
      Serial.println("UI:HOME");
      break;
    case 'm':
    case 'M':
      headUnitHmiOpenMap();
      Serial.println("UI:MAP");
      break;
    case 's':
    case 'S':
      headUnitHmiOpenSetting();
      Serial.println("UI:SETTING");
      break;
    case 'i':
    case 'I':
      headUnitHmiOpenSettingInfo();
      Serial.println("UI:SETTING_INFO");
      break;
    default:
      break;
  }
}

void headUnitInputTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    while (Serial.available() > 0) {
      handleDebugInput((char)Serial.read());
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_TASK_INPUT_MS));
  }
}
