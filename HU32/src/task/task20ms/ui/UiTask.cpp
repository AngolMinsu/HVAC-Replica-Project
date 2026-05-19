#include "UiTask.h"

#include "../../../hmi/HuRtosContext.h"

static uint8_t copyStateForRender(HuRtosContext* context, SystemState& out, uint32_t& dirtyFlags) {
  if (xSemaphoreTake(context->stateMutex, pdMS_TO_TICKS(2)) != pdTRUE) {
    return 0;
  }

  dirtyFlags = context->state->dirtyFlags;
  if (dirtyFlags != DIRTY_NONE) {
    out = *context->state;
    context->state->dirtyFlags = DIRTY_NONE;
  }

  xSemaphoreGive(context->stateMutex);
  return dirtyFlags != DIRTY_NONE;
}

void huUiTask(void* parameter) {
  HuRtosContext* context = static_cast<HuRtosContext*>(parameter);
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    SystemState snapshot;
    uint32_t dirtyFlags = DIRTY_NONE;
    if (copyStateForRender(context, snapshot, dirtyFlags)) {
      context->uiManager->render(snapshot, dirtyFlags);
    }

    context->uiManager->loop();
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_UI_FRAME_MS));
  }
}
