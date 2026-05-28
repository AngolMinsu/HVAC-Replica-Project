#include "TaskInit.h"

#include <Arduino.h>

#include "../driver/DisplayDriver.h"
#include "../hmi/HeadUnitHmi.h"

void taskInitBegin() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("HU7 start");

  if (!displayDriverBegin()) {
    Serial.println("Display driver init failed");
    while (true) {
      delay(1000);
    }
  }

  headUnitHmiBegin();
  Serial.println("HU7 UI ready");
}
