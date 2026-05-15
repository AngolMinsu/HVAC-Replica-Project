#include "../../../MKBD/encoder/EncoderInput.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_EncoderInput(uint16_t loop) {
  EncoderHistory history;
  initEncoderHistory(history);
  ASSERT_EQUALS(0, history.prevDriverA, HIGH);
  ASSERT_EQUALS(1, history.prevPassengerA, HIGH);

  EncoderLevels levels = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
  ASSERT_EQUALS(2, detectEncoderEvent(history, levels, 100, 60), ENCODER_EVENT_NONE);

  levels.driverA = LOW;
  levels.driverB = HIGH;
  ASSERT_EQUALS(3, detectEncoderEvent(history, levels, 200, 60), ENCODER_EVENT_DRIVER_CW);

  levels.driverA = HIGH;
  ASSERT_EQUALS(4, detectEncoderEvent(history, levels, 270, 60), ENCODER_EVENT_NONE);
  levels.driverA = LOW;
  levels.driverB = LOW;
  ASSERT_EQUALS(5, detectEncoderEvent(history, levels, 340, 60), ENCODER_EVENT_DRIVER_CCW);

  levels.driverA = HIGH;
  levels.driverB = HIGH;
  levels.driverSw = LOW;
  ASSERT_EQUALS(6, detectEncoderEvent(history, levels, 410, 60), ENCODER_EVENT_DRIVER_SW);

  levels.driverSw = HIGH;
  levels.passengerA = LOW;
  levels.passengerB = HIGH;
  ASSERT_EQUALS(7, detectEncoderEvent(history, levels, 480, 60), ENCODER_EVENT_PASSENGER_CW);

  levels.passengerA = HIGH;
  ASSERT_EQUALS(8, detectEncoderEvent(history, levels, 550, 60), ENCODER_EVENT_NONE);
  levels.passengerA = LOW;
  levels.passengerB = LOW;
  ASSERT_EQUALS(9, detectEncoderEvent(history, levels, 620, 60), ENCODER_EVENT_PASSENGER_CCW);

  levels.passengerA = HIGH;
  levels.passengerB = HIGH;
  levels.passengerSw = LOW;
  ASSERT_EQUALS(10, detectEncoderEvent(history, levels, 690, 60), ENCODER_EVENT_PASSENGER_SW);
  return 1;
}
