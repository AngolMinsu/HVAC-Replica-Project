#include "Task10msUi.h"

#include "../driver/DisplayDriver.h"
#include "../hmi/HeadUnitHmi.h"

void task10msUiMain() {
  headUnitHmiTick();
  displayDriverLoop();
}
