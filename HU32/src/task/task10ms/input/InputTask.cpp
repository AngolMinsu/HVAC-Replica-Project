#include "InputTask.h"

#include "../../../hmi/HuRtosContext.h"

static void sendUiEvent(HuRtosContext* context, UiEventType type, uint8_t value) {
  UiEvent event;
  event.type = type;
  event.value = value;
  xQueueSend(context->uiEventQueue, &event, 0);
}

void huInputTask(void* parameter) {
  HuRtosContext* context = static_cast<HuRtosContext*>(parameter);

  for (;;) {
    if (Serial.available()) {
      char ch = (char)Serial.read();
      switch (ch) {
        case 'n': sendUiEvent(context, UI_EVENT_FOCUS_NEXT, 0); break;
        case 'p': sendUiEvent(context, UI_EVENT_FOCUS_PREV, 0); break;
        case 's': sendUiEvent(context, UI_EVENT_SELECT, 0); break;
        case 'b': sendUiEvent(context, UI_EVENT_BACK, 0); break;
        case 'h': sendUiEvent(context, UI_EVENT_HOME, 0); break;
        case 'm': sendUiEvent(context, UI_EVENT_OPEN_MEDIA, 0); break;
        case 'c': sendUiEvent(context, UI_EVENT_OPEN_HVAC, 0); break;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
