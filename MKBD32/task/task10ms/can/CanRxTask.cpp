#include "CanRxTask.h"

#include "../../MkbdTaskHooks.h"

void mkbdTask10msCanRxRun() {
  if (processCanReceive()) {
    printSystemStatus(state);
  }
}
