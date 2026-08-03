#include <stdint.h>
#include <stdio.h>

#include "TestSupport/Test_Assert.h"

uint8_t Test_CanPayloadCreate();
uint8_t Test_CanChecksum();
uint8_t Test_CanSerialization();
uint8_t Test_StateInitialization();
uint8_t Test_ScreenToggle();
uint8_t Test_StateTextConversion();
uint8_t Test_ButtonInput();
uint8_t Test_EncoderInput();
uint8_t Test_Datc();
uint8_t Test_Info();
uint8_t Test_AppLogic();
uint8_t Test_CanHandler();
uint8_t Test_CanDriver();
uint8_t Test_CanMonitor();
uint8_t Test_MkbdCanService();
uint8_t Test_MkbdHardware();
uint8_t Test_TaskModules();
uint8_t Test_MkbdRtos();

int main() {
  puts("[TEST] MKBD unit test start");
  ASSERT(Test_CanPayloadCreate());
  ASSERT(Test_CanChecksum());
  ASSERT(Test_CanSerialization());
  ASSERT(Test_StateInitialization());
  ASSERT(Test_ScreenToggle());
  ASSERT(Test_StateTextConversion());
  ASSERT(Test_ButtonInput());
  ASSERT(Test_EncoderInput());
  ASSERT(Test_Datc());
  ASSERT(Test_Info());
  ASSERT(Test_AppLogic());
  ASSERT(Test_CanHandler());
  ASSERT(Test_CanDriver());
  ASSERT(Test_CanMonitor());
  ASSERT(Test_MkbdCanService());
  ASSERT(Test_MkbdHardware());
  ASSERT(Test_TaskModules());
  ASSERT(Test_MkbdRtos());
  puts("[PASS] MKBD module test suite");
  return 0;
}
