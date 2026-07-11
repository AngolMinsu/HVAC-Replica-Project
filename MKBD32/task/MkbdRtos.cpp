#include "MkbdRtos.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>


#include "task10ms/can/CanRxTask.h"
#include "task10ms/input/InputTask.h"
#include "task10ms/output/OutputTask.h"
#include "task100ms/display/DisplayTask.h"

static SemaphoreHandle_t mkbdTaskMutex = nullptr;

static void runLocked(void (*taskFn)()) {
  if (mkbdTaskMutex == nullptr) {
    taskFn();
    return;
  }

  if (xSemaphoreTake(mkbdTaskMutex, portMAX_DELAY) == pdTRUE) {
    taskFn();
    xSemaphoreGive(mkbdTaskMutex);
  }
}

static void task10msInput(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    runLocked(mkbdTask10msInputRun);
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

static void task10msCanRx(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    runLocked(mkbdTask10msCanRxRun);
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

static void task10msOutput(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    runLocked(mkbdTask10msOutputRun);
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

static void task100msDisplay(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    runLocked(mkbdTask100msDisplayRun);
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(100));
  }
}

void mkbdRtosStart() {
  if (mkbdTaskMutex == nullptr) {
    mkbdTaskMutex = xSemaphoreCreateMutex();
  }

  xTaskCreatePinnedToCore(task10msCanRx, "MKBD_CAN_RX", 4096, nullptr, 4, nullptr, 0);
  xTaskCreatePinnedToCore(task10msInput, "MKBD_INPUT", 4096, nullptr, 3, nullptr, 1);
  xTaskCreatePinnedToCore(task10msOutput, "MKBD_OUTPUT", 2048, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(task100msDisplay, "MKBD_DISPLAY", 4096, nullptr, 1, nullptr, 1);
}

void mkbdRtosIdle() {
  vTaskDelay(portMAX_DELAY);
}

