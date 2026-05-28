#include <Arduino.h>

#include "src/hmi/HeadUnitApp.h"

void setup() {
  headUnitAppBegin();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
