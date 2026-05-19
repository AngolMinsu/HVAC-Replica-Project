#include "SystemTask.h"

#include "../../../can/CanProtocol.h"
#include "../../../hmi/HuRtosContext.h"

static void handleUiEvent(SystemState& state, const UiEvent& event);
static void handleMkbdCanUiControl(SystemState& state, const CanPayload& payload);
static void moveFocusNext(SystemState& state);
static void moveFocusPrev(SystemState& state);
static void updateSystemStats(SystemState& state);
static void updatePressedTransition(SystemState& state);
static void markDirtyForSignal(SystemState& state, uint8_t signal);

void huSystemTask(void* parameter) {
  HuRtosContext* context = static_cast<HuRtosContext*>(parameter);
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    if (xSemaphoreTake(context->stateMutex, portMAX_DELAY) == pdTRUE) {
      HuStateEvent canEvent;
      while (xQueueReceive(context->canRxQueue, &canEvent, 0) == pdTRUE) {
        context->state->lastCanId = canEvent.canId;
        context->state->lastCanRxMs = millis();
        context->state->canRxOk = 1;
        applyCanPayloadToState(*context->state, canEvent.payload);
        handleMkbdCanUiControl(*context->state, canEvent.payload);
        markDirtyForSignal(*context->state, canEvent.payload.signal);
      }

      UiEvent uiEvent;
      while (xQueueReceive(context->uiEventQueue, &uiEvent, 0) == pdTRUE) {
        handleUiEvent(*context->state, uiEvent);
      }

      AssetReadyEvent assetEvent;
      while (xQueueReceive(context->assetEventQueue, &assetEvent, 0) == pdTRUE) {
        context->state->assetsReady = assetEvent.type == ASSET_EVENT_READY ? 1 : 0;
        context->state->dirtyFlags |= DIRTY_FULL;
      }

      updatePressedTransition(*context->state);
      updateSystemStats(*context->state);
      xSemaphoreGive(context->stateMutex);
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

static void handleUiEvent(SystemState& state, const UiEvent& event) {
  switch (event.type) {
    case UI_EVENT_FOCUS_NEXT:
      moveFocusNext(state);
      break;

    case UI_EVENT_FOCUS_PREV:
      moveFocusPrev(state);
      break;

    case UI_EVENT_SELECT:
      if (state.screen == HU_SCREEN_HOME) {
        state.panelVisualState = HU_PANEL_PRESSED;
        state.panelPressedUntilMs = millis() + 120;
        state.dirtyFlags |= DIRTY_FOCUS | DIRTY_HOME;
      }
      break;

    case UI_EVENT_BACK:
    case UI_EVENT_HOME:
      state.screen = HU_SCREEN_HOME;
      state.dirtyFlags |= DIRTY_FULL;
      break;

    case UI_EVENT_OPEN_MEDIA:
      state.screen = HU_SCREEN_MEDIA;
      state.dirtyFlags |= DIRTY_FULL;
      break;

    case UI_EVENT_OPEN_MAP:
      state.screen = HU_SCREEN_MAP;
      state.dirtyFlags |= DIRTY_FULL;
      break;

    case UI_EVENT_OPEN_HVAC:
      state.screen = HU_SCREEN_HVAC;
      state.dirtyFlags |= DIRTY_FULL;
      break;

    case UI_EVENT_OPEN_SETTING:
      state.screen = HU_SCREEN_SETTING;
      state.focusedSettingTile = 0;
      state.panelVisualState = HU_PANEL_FOCUSED;
      state.dirtyFlags |= DIRTY_FULL;
      break;

    default:
      break;
  }
}

static void moveFocusNext(SystemState& state) {
  if (state.screen == HU_SCREEN_SETTING) {
    state.focusedSettingTile = (state.focusedSettingTile + 1) % 6;
    state.panelVisualState = HU_PANEL_FOCUSED;
    state.dirtyFlags |= DIRTY_FOCUS | DIRTY_SETTING;
    return;
  }

  if (state.screen == HU_SCREEN_HOME) {
    state.focusedPanel = (HuPanelFocus)((state.focusedPanel + 1) % 3);
    state.panelVisualState = HU_PANEL_FOCUSED;
    state.dirtyFlags |= DIRTY_FOCUS | DIRTY_HOME;
  }
}

static void moveFocusPrev(SystemState& state) {
  if (state.screen == HU_SCREEN_SETTING) {
    state.focusedSettingTile = (state.focusedSettingTile + 5) % 6;
    state.panelVisualState = HU_PANEL_FOCUSED;
    state.dirtyFlags |= DIRTY_FOCUS | DIRTY_SETTING;
    return;
  }

  if (state.screen == HU_SCREEN_HOME) {
    state.focusedPanel = (HuPanelFocus)((state.focusedPanel + 2) % 3);
    state.panelVisualState = HU_PANEL_FOCUSED;
    state.dirtyFlags |= DIRTY_FOCUS | DIRTY_HOME;
  }
}

static void handleMkbdCanUiControl(SystemState& state, const CanPayload& payload) {
  if (!canValidateChecksum(payload) || payload.result == CAN_RESULT_FAIL) {
    return;
  }

  if (payload.signal == CAN_SIGNAL_HU_OPEN_HOME && payload.value == 1) {
    UiEvent event = { UI_EVENT_HOME, 0 };
    handleUiEvent(state, event);
    return;
  }

  if (payload.signal == CAN_SIGNAL_HU_OPEN_MAP && payload.value == 1) {
    UiEvent event = { UI_EVENT_OPEN_MAP, 0 };
    handleUiEvent(state, event);
    return;
  }

  if (payload.signal == CAN_SIGNAL_HU_OPEN_MEDIA && payload.value == 1) {
    UiEvent event = { UI_EVENT_OPEN_MEDIA, 0 };
    handleUiEvent(state, event);
    return;
  }

  if (payload.signal == CAN_SIGNAL_HU_FOCUS_NEXT && payload.value == 1) {
    UiEvent event = { UI_EVENT_FOCUS_NEXT, 0 };
    handleUiEvent(state, event);
    return;
  }

  if (payload.signal == CAN_SIGNAL_HU_FOCUS_PREV && payload.value == 1) {
    UiEvent event = { UI_EVENT_FOCUS_PREV, 0 };
    handleUiEvent(state, event);
    return;
  }

  if (payload.signal == CAN_SIGNAL_PASSENGER_ENCODER_SW && payload.value == 1) {
    UiEvent event = { UI_EVENT_SELECT, 0 };
    handleUiEvent(state, event);
  }
}

static void updatePressedTransition(SystemState& state) {
  if (state.panelVisualState != HU_PANEL_PRESSED || millis() < state.panelPressedUntilMs) {
    return;
  }

  if (state.focusedPanel == HU_PANEL_MEDIA) {
    state.screen = HU_SCREEN_MEDIA;
  } else if (state.focusedPanel == HU_PANEL_MAP) {
    state.screen = HU_SCREEN_MAP;
  } else {
    state.screen = HU_SCREEN_SETTING;
  }

  state.panelVisualState = HU_PANEL_FOCUSED;
  state.dirtyFlags |= DIRTY_FULL;
}

static void updateSystemStats(SystemState& state) {
  uint32_t now = millis();
  uint8_t rxOk = now - state.lastCanRxMs <= GDS_CAN_TIMEOUT_MS;
  if (state.canRxOk != rxOk) {
    state.canRxOk = rxOk;
    state.dirtyFlags |= DIRTY_STATUS | DIRTY_TOP_BAR | DIRTY_BOTTOM_BAR;
  }

  state.freeHeap = ESP.getFreeHeap();
  state.frameCounter++;
  if (now - state.lastFpsMs >= 1000) {
    state.fps = state.frameCounter;
    state.frameCounter = 0;
    state.lastFpsMs = now;
    if (state.screen == HU_SCREEN_SETTING) {
      state.dirtyFlags |= DIRTY_SETTING;
    }
  }
}

static void markDirtyForSignal(SystemState& state, uint8_t signal) {
  state.dirtyFlags |= DIRTY_STATUS | DIRTY_TOP_BAR | DIRTY_BOTTOM_BAR;

  switch (signal) {
    case CAN_SIGNAL_FAN_SPEED:
    case CAN_SIGNAL_TEMPERATURE:
    case CAN_SIGNAL_PASSENGER_TEMPERATURE:
    case CAN_SIGNAL_MODE:
    case CAN_SIGNAL_AC:
    case CAN_SIGNAL_AUTO:
    case CAN_SIGNAL_POWER:
      state.dirtyFlags |= DIRTY_HVAC | DIRTY_HOME;
      break;

    case CAN_SIGNAL_MEDIA:
    case CAN_SIGNAL_VOLUME:
    case CAN_SIGNAL_MUTE:
    case CAN_SIGNAL_MEDIA_MODE:
    case CAN_SIGNAL_MEDIA_INDEX:
    case CAN_SIGNAL_HU_FOCUS_PREV:
    case CAN_SIGNAL_HU_FOCUS_NEXT:
    case CAN_SIGNAL_HU_OPEN_HOME:
    case CAN_SIGNAL_HU_OPEN_MAP:
    case CAN_SIGNAL_HU_OPEN_MEDIA:
      state.dirtyFlags |= DIRTY_MEDIA | DIRTY_HOME;
      break;

    case CAN_SIGNAL_MAP:
    case CAN_SIGNAL_HOME:
      state.dirtyFlags |= DIRTY_MAP | DIRTY_HOME;
      break;

    default:
      state.dirtyFlags |= dirtyForScreen(state.screen);
      break;
  }
}
