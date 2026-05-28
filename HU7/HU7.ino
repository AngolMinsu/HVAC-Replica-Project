#include "src/task/TaskInit.h"
#include "src/task/Task10msUi.h"

void setup() {
  taskInitBegin();
}

void loop() {
  task10msUiMain();
}
