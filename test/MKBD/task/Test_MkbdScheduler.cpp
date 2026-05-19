#include "../../../MKBD/task/MkbdScheduler.h"
#include "../../../MKBD/app/AppLogic.h"
#include "../../../MKBD/button/ButtonInput.h"
#include "../../../MKBD/encoder/EncoderInput.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

unsigned long testArduinoMillis = 0;
SystemState state;
unsigned long lastDisplayTime = 0;

static uint8_t nextButtonEvent = APP_BUTTON_NONE;
static uint8_t nextEncoderEvent = ENCODER_EVENT_NONE;
static uint8_t nextCanReceive = 0;

static uint8_t buttonReadCount = 0;
static uint8_t encoderReadCount = 0;
static uint8_t fanUpdateCount = 0;
static uint8_t displayDrawCount = 0;
static uint8_t statusPrintCount = 0;
static uint8_t canReceiveCount = 0;
static uint8_t buttonMenuBroadcastCount = 0;
static uint8_t encoderSwitchBroadcastCount = 0;
static uint8_t changedBroadcastCount = 0;

static void resetTaskCounters() {
  nextButtonEvent = APP_BUTTON_NONE;
  nextEncoderEvent = ENCODER_EVENT_NONE;
  nextCanReceive = 0;
  buttonReadCount = 0;
  encoderReadCount = 0;
  fanUpdateCount = 0;
  displayDrawCount = 0;
  statusPrintCount = 0;
  canReceiveCount = 0;
  buttonMenuBroadcastCount = 0;
  encoderSwitchBroadcastCount = 0;
  changedBroadcastCount = 0;
  lastDisplayTime = 0;
  initSystemState(state);
  testSetMillis(0);
}

uint8_t readButtonEvent() {
  buttonReadCount++;
  uint8_t event = nextButtonEvent;
  nextButtonEvent = APP_BUTTON_NONE;
  return event;
}

uint8_t readEncoderEvent() {
  encoderReadCount++;
  uint8_t event = nextEncoderEvent;
  nextEncoderEvent = ENCODER_EVENT_NONE;
  return event;
}

int updateFanMotor() {
  fanUpdateCount++;
  return 1;
}

uint8_t drawCurrentScreen() {
  displayDrawCount++;
  return 1;
}

uint8_t printSystemStatus(const SystemState&) {
  statusPrintCount++;
  return 1;
}

uint8_t processCanReceive() {
  canReceiveCount++;
  uint8_t result = nextCanReceive;
  nextCanReceive = 0;
  return result;
}

uint8_t broadcastButtonMenuEvent(uint8_t, const SystemState&) {
  buttonMenuBroadcastCount++;
  return 0;
}

uint8_t broadcastEncoderSwitchEvent(uint8_t, const SystemState&) {
  encoderSwitchBroadcastCount++;
  return 1;
}

uint8_t broadcastChangedHvacStatus(const SystemState&, const SystemState&) {
  changedBroadcastCount++;
  return 1;
}

uint8_t Test_MkbdScheduler(uint16_t loop) {
  if (loop == 0) loop = 1;

  for (uint16_t i = 0; i < loop; i++) {
    resetTaskCounters();
    mkbdSchedulerRun();
    ASSERT_EQUALS(0, buttonReadCount, 0);
    ASSERT_EQUALS(1, canReceiveCount, 0);
    ASSERT_EQUALS(2, fanUpdateCount, 0);
    ASSERT_EQUALS(3, displayDrawCount, 0);

    testSetMillis(10);
    nextButtonEvent = APP_BUTTON_FAN_UP;
    mkbdSchedulerRun();
    ASSERT_EQUALS(4, buttonReadCount, 1);
    ASSERT_EQUALS(5, encoderReadCount, 1);
    ASSERT_EQUALS(6, canReceiveCount, 1);
    ASSERT_EQUALS(7, fanUpdateCount, 1);
    ASSERT_EQUALS(8, state.fanSpeed, GDS_FAN_SPEED_MIN + 1);
    ASSERT_EQUALS(9, changedBroadcastCount, 1);
    ASSERT_EQUALS(10, displayDrawCount, 0);

    testSetMillis(GDS_DISPLAY_INTERVAL_MS);
    nextCanReceive = 1;
    mkbdSchedulerRun();
    ASSERT_EQUALS(11, canReceiveCount, 2);
    ASSERT_EQUALS(12, statusPrintCount >= 2, 1);
    ASSERT_EQUALS(13, displayDrawCount, 1);
    ASSERT_EQUALS(14, lastDisplayTime, GDS_DISPLAY_INTERVAL_MS);

    testSetMillis(GDS_DISPLAY_INTERVAL_MS + 10);
    nextEncoderEvent = ENCODER_EVENT_DRIVER_CW;
    mkbdSchedulerRun();
    ASSERT_EQUALS(15, encoderSwitchBroadcastCount, 1);
    ASSERT_EQUALS(16, state.driverTemp, GDS_TEMP_DEFAULT + 1);
    ASSERT_EQUALS(17, displayDrawCount, 1);
  }

  return 1;
}
