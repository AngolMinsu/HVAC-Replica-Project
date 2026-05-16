#include "HuRtosApp.h"

#include "../can/CanDriver.h"
#include "../can/CanMonitor.h"
#include "../can/CanProtocol.h"
#include "../../GDS.h"
#include "AssetManager.h"
#include "HuTypes.h"
#include "UIManager.h"

static QueueHandle_t canRxQueue = nullptr;
static QueueHandle_t canTxQueue = nullptr;
static QueueHandle_t uiEventQueue = nullptr;
static QueueHandle_t assetEventQueue = nullptr;
static SemaphoreHandle_t stateMutex = nullptr;

static SystemState systemState;
static AssetManager assetManager;
static UIManager uiManager;
static uint8_t huTxCounter = 0;

static void canRxTask(void* parameter);
static void canTxTask(void* parameter);
static void inputTask(void* parameter);
static void assetTask(void* parameter);
static void uiTask(void* parameter);
static void systemTask(void* parameter);

static void sendUiEvent(UiEventType type, uint8_t value);
static void handleUiEvent(SystemState& state, const UiEvent& event);
static void updateSystemStats(SystemState& state);
static void updatePressedTransition(SystemState& state);
static void markDirtyForSignal(SystemState& state, uint8_t signal);
static uint8_t copyStateForRender(SystemState& out, uint32_t& dirtyFlags);

void huRtosAppBegin() {
  initSystemState(systemState);

  canRxQueue = xQueueCreate(12, sizeof(HuStateEvent));
  canTxQueue = xQueueCreate(8, sizeof(HuCanCommand));
  uiEventQueue = xQueueCreate(12, sizeof(UiEvent));
  assetEventQueue = xQueueCreate(2, sizeof(AssetReadyEvent));
  stateMutex = xSemaphoreCreateMutex();

  uint8_t ready = canDriverBegin(GDS_PIN_CAN_CS);
  systemState.canReady = ready;
  Serial.print("CAN:");
  Serial.println(ready ? "READY" : "FAIL");

  uiManager.begin(&assetManager);

  xTaskCreatePinnedToCore(canRxTask, "CAN_RX", 4096, nullptr, 4, nullptr, 0);
  xTaskCreatePinnedToCore(canTxTask, "CAN_TX", 4096, nullptr, 3, nullptr, 0);
  xTaskCreatePinnedToCore(systemTask, "SYSTEM", 6144, nullptr, 3, nullptr, 0);
  xTaskCreatePinnedToCore(assetTask, "ASSET", 6144, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(inputTask, "INPUT", 4096, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(uiTask, "UI", 8192, nullptr, 2, nullptr, 1);

  Serial.println("HU32 RTOS app started");
  Serial.println("Input: n/p focus, s select, b back, h home, m media, c hvac");
}

static void canRxTask(void* parameter) {
  (void)parameter;
  TickType_t delayTicks = pdMS_TO_TICKS(5);

  for (;;) {
    CanFrame frame;
    if (canDriverReceive(frame)) {
      canMonitorPrintFrame("RX", frame);

      if (frame.dlc == CAN_DLC && (frame.id == CAN_ID_CONTROL_RESPONSE || frame.id == CAN_ID_HVAC_STATUS)) {
        HuStateEvent event;
        event.canId = frame.id;
        event.payload = canPayloadFromBytes(frame.data);
        event.rxTick = xTaskGetTickCount();
        canMonitorPrintPayloadSummary(event.payload);
        xQueueSend(canRxQueue, &event, 0);
      }
    }

    vTaskDelay(delayTicks);
  }
}

static void canTxTask(void* parameter) {
  (void)parameter;

  for (;;) {
    HuCanCommand command;
    if (xQueueReceive(canTxQueue, &command, portMAX_DELAY) == pdTRUE) {
      CanPayload payload = canMakePayload(command.service, CAN_RESULT_NORMAL, command.signal, command.value, 0, huTxCounter++);
      CanFrame frame;
      frame.id = CAN_ID_CONTROL_REQUEST;
      frame.dlc = CAN_DLC;
      canPayloadToBytes(payload, frame.data);

      canMonitorPrintFrame("TX", frame);
      canMonitorPrintPayloadSummary(payload);
      canDriverSend(frame);
    }
  }
}

static void inputTask(void* parameter) {
  (void)parameter;

  for (;;) {
    if (Serial.available()) {
      char ch = (char)Serial.read();
      switch (ch) {
        case 'n': sendUiEvent(UI_EVENT_FOCUS_NEXT, 0); break;
        case 'p': sendUiEvent(UI_EVENT_FOCUS_PREV, 0); break;
        case 's': sendUiEvent(UI_EVENT_SELECT, 0); break;
        case 'b': sendUiEvent(UI_EVENT_BACK, 0); break;
        case 'h': sendUiEvent(UI_EVENT_HOME, 0); break;
        case 'm': sendUiEvent(UI_EVENT_OPEN_MEDIA, 0); break;
        case 'c': sendUiEvent(UI_EVENT_OPEN_HVAC, 0); break;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void assetTask(void* parameter) {
  (void)parameter;

  AssetReadyEvent event;
  event.type = assetManager.begin() ? ASSET_EVENT_READY : ASSET_EVENT_FAIL;
  xQueueSend(assetEventQueue, &event, portMAX_DELAY);

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void uiTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    SystemState snapshot;
    uint32_t dirtyFlags = DIRTY_NONE;
    if (copyStateForRender(snapshot, dirtyFlags)) {
      uiManager.render(snapshot, dirtyFlags);
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_UI_FRAME_MS));
  }
}

static void systemTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    if (xSemaphoreTake(stateMutex, portMAX_DELAY) == pdTRUE) {
      HuStateEvent canEvent;
      while (xQueueReceive(canRxQueue, &canEvent, 0) == pdTRUE) {
        systemState.lastCanId = canEvent.canId;
        systemState.lastCanRxMs = millis();
        systemState.canRxOk = 1;
        applyCanPayloadToState(systemState, canEvent.payload);
        markDirtyForSignal(systemState, canEvent.payload.signal);
      }

      UiEvent uiEvent;
      while (xQueueReceive(uiEventQueue, &uiEvent, 0) == pdTRUE) {
        handleUiEvent(systemState, uiEvent);
      }

      AssetReadyEvent assetEvent;
      while (xQueueReceive(assetEventQueue, &assetEvent, 0) == pdTRUE) {
        systemState.assetsReady = assetEvent.type == ASSET_EVENT_READY ? 1 : 0;
        systemState.dirtyFlags |= DIRTY_FULL;
      }

      updatePressedTransition(systemState);
      updateSystemStats(systemState);
      xSemaphoreGive(stateMutex);
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

static void sendUiEvent(UiEventType type, uint8_t value) {
  UiEvent event;
  event.type = type;
  event.value = value;
  xQueueSend(uiEventQueue, &event, 0);
}

static void handleUiEvent(SystemState& state, const UiEvent& event) {
  switch (event.type) {
    case UI_EVENT_FOCUS_NEXT:
      state.focusedPanel = (HuPanelFocus)((state.focusedPanel + 1) % 3);
      state.panelVisualState = HU_PANEL_FOCUSED;
      state.dirtyFlags |= DIRTY_FOCUS | DIRTY_HOME;
      break;

    case UI_EVENT_FOCUS_PREV:
      state.focusedPanel = (HuPanelFocus)((state.focusedPanel + 2) % 3);
      state.panelVisualState = HU_PANEL_FOCUSED;
      state.dirtyFlags |= DIRTY_FOCUS | DIRTY_HOME;
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
      state.dirtyFlags |= DIRTY_FULL;
      break;

    default:
      break;
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

static uint8_t copyStateForRender(SystemState& out, uint32_t& dirtyFlags) {
  if (xSemaphoreTake(stateMutex, pdMS_TO_TICKS(2)) != pdTRUE) {
    return 0;
  }

  dirtyFlags = systemState.dirtyFlags;
  if (dirtyFlags != DIRTY_NONE) {
    out = systemState;
    systemState.dirtyFlags = DIRTY_NONE;
  }

  xSemaphoreGive(stateMutex);
  return dirtyFlags != DIRTY_NONE;
}
