#ifndef HU_RTOS_CONTEXT_H
#define HU_RTOS_CONTEXT_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

#include "AssetManager.h"
#include "HuTypes.h"
#include "UIManager.h"

struct HuRtosContext {
  QueueHandle_t canRxQueue;
  QueueHandle_t canTxQueue;
  QueueHandle_t uiEventQueue;
  QueueHandle_t assetEventQueue;
  SemaphoreHandle_t stateMutex;

  SystemState* state;
  AssetManager* assetManager;
  UIManager* uiManager;
  uint8_t* huTxCounter;
};

#endif
