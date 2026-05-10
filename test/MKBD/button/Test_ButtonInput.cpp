#include "../../../MKBD/button/ButtonInput.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_ButtonInput(uint16_t loop) {
  ButtonHistory history;
  initButtonHistory(history);
  ASSERT_EQUALS(0, history.prevBtn2, HIGH);
  ASSERT_EQUALS(1, history.lastEventTime, 0);

  ButtonLevels levels = {HIGH, HIGH, HIGH, HIGH, HIGH};
  ASSERT_EQUALS(2, detectButtonEvent(history, levels, 100, 60), APP_BUTTON_NONE);

  levels.btn2 = LOW;
  ASSERT_EQUALS(3, detectButtonEvent(history, levels, 200, 60), APP_BUTTON_2);
  ASSERT_EQUALS(4, history.lastEventTime, 200);

  levels.btn3 = LOW;
  ASSERT_EQUALS(5, detectButtonEvent(history, levels, 260, 60), APP_BUTTON_NONE);
  ASSERT_EQUALS(6, history.lastEventTime, 200);
  ASSERT_EQUALS(7, detectButtonEvent(history, levels, 261, 60), APP_BUTTON_NONE);

  levels.screen = LOW;
  levels.btn2 = HIGH;
  levels.btn3 = HIGH;
  ASSERT_EQUALS(8, detectButtonEvent(history, levels, 330, 60), APP_BUTTON_SCREEN);

  levels.screen = HIGH;
  ASSERT_EQUALS(9, detectButtonEvent(history, levels, 400, 60), APP_BUTTON_NONE);
  levels.btn5 = LOW;
  ASSERT_EQUALS(10, detectButtonEvent(history, levels, 470, 60), APP_BUTTON_5);
  return 1;
}
