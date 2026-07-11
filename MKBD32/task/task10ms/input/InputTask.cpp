#include "InputTask.h"

#include "../../../app/AppLogic.h"
#include "../../../button/ButtonInput.h"
#include "../../../encoder/EncoderInput.h"
#include "../../MkbdTaskHooks.h"

void mkbdTask10msInputRun() {
  uint8_t button = readButtonEvent();
  if (button != APP_BUTTON_NONE) {
    SystemState before = state;
    uint8_t changed = handleButtonAction(state, button);
    uint8_t menuSent = broadcastButtonMenuEvent(button, before);
    if (changed && !menuSent) {
      printSystemStatus(state);
      broadcastChangedHvacStatus(before, state);
    }
  }

  uint8_t encoderEvent = readEncoderEvent();
  if (encoderEvent != ENCODER_EVENT_NONE) {
    SystemState before = state;
    uint8_t changed = handleEncoderAction(state, encoderEvent);
    if (changed) {
      printSystemStatus(state);
      broadcastChangedHvacStatus(before, state);
    }
    broadcastEncoderSwitchEvent(encoderEvent, before);
  }
}
