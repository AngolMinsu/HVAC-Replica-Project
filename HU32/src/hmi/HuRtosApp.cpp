#include "HuRtosApp.h"

#include "../can/CanDriver.h"
#include "../../GDS.h"
#include "../task/task10ms/can/CanRxTask.h"
#include "../task/task10ms/can/CanTxTask.h"
#include "../task/task10ms/input/InputTask.h"
#include "../task/task20ms/ui/UiTask.h"
#include "../task/task10ms/system/SystemTask.h"
#include "AssetManager.h"
#include "HuRtosContext.h"
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
static HuRtosContext context;

static void initContext() {
  context.canRxQueue = canRxQueue;
  context.canTxQueue = canTxQueue;
  context.uiEventQueue = uiEventQueue;
  context.assetEventQueue = assetEventQueue;
  context.stateMutex = stateMutex;
  context.state = &systemState;
  context.assetManager = &assetManager;
  context.uiManager = &uiManager;
  context.huTxCounter = &huTxCounter;
}

void huRtosAppBegin() {
  Serial.begin(GDS_SERIAL_BAUD);
  delay(1000);
  Serial.println();
  Serial.println("HU32 Head Unit start");

  initSystemState(systemState);

  canRxQueue = xQueueCreate(12, sizeof(HuStateEvent));
  canTxQueue = xQueueCreate(8, sizeof(HuCanCommand));
  uiEventQueue = xQueueCreate(12, sizeof(UiEvent));
  assetEventQueue = xQueueCreate(2, sizeof(AssetReadyEvent));
  stateMutex = xSemaphoreCreateMutex();
  initContext();

  uint8_t ready = GDS_CAN_ENABLED ? canDriverBegin(GDS_PIN_CAN_CS) : 0;
  systemState.canReady = ready;
  Serial.print("CAN:");
  Serial.println(GDS_CAN_ENABLED ? (ready ? "READY" : "FAIL") : "SKIP");
  Serial.flush();

  uiManager.begin(&assetManager);

  systemState.assetsReady = assetManager.begin() ? 1 : 0;
  systemState.dirtyFlags |= DIRTY_FULL;
  Serial.flush();

  xTaskCreatePinnedToCore(huCanRxTask, "CAN_RX", 8192, &context, 4, nullptr, 0);
  xTaskCreatePinnedToCore(huCanTxTask, "CAN_TX", 4096, &context, 3, nullptr, 0);
  xTaskCreatePinnedToCore(huSystemTask, "SYSTEM", 6144, &context, 3, nullptr, 0);
  xTaskCreatePinnedToCore(huInputTask, "INPUT", 4096, &context, 2, nullptr, 1);
  xTaskCreatePinnedToCore(huUiTask, "UI", 8192, &context, 2, nullptr, 1);

  Serial.println("HU32 ready");
  Serial.print("CAN status after boot:");
  Serial.println(systemState.canReady ? "READY" : "FAIL");
}
