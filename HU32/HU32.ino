#include <Arduino.h>
#include "src/hmi/HuRtosApp.h"

void setup() {
  huRtosAppBegin();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
