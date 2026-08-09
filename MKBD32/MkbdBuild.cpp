// Arduino IDE compiles root .cpp files. Subfolder sources are included here
// so MKBD.ino can stay as the application entry point only.
#include "state/State.cpp"
#include "display/Datc.cpp"
#include "display/Info.cpp"
#include "app/AppLogic.cpp"
#include "app/MkbdHardware.cpp"
#include "button/ButtonInput.cpp"
#include "encoder/EncoderInput.cpp"
#include "can/CanProtocol.cpp"
#include "can/CanHandler.cpp"
#include "can/CanMonitor.cpp"
#include "can/CanDriver.cpp"
#ifdef ARDUINO
#include "ota/CanOtaReceiver.cpp"
#endif
#include "can/MkbdCanService.cpp"
#include "task/MkbdRtos.cpp"
#include "task/task10ms/input/InputTask.cpp"
#include "task/task10ms/can/CanRxTask.cpp"
#include "task/task10ms/output/OutputTask.cpp"
#include "task/task100ms/display/DisplayTask.cpp"
