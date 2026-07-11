#include "CanDriver.h"

#include <driver/twai.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static uint8_t canReady = 0;
static esp_err_t lastCanError = ESP_OK;

static const char* canErrorToText(esp_err_t error) {
  switch (error) {
    case ESP_OK: return "ESP_OK";
    case ESP_ERR_INVALID_ARG: return "ESP_ERR_INVALID_ARG";
    case ESP_ERR_INVALID_STATE: return "ESP_ERR_INVALID_STATE";
    case ESP_ERR_NO_MEM: return "ESP_ERR_NO_MEM";
    case ESP_ERR_TIMEOUT: return "ESP_ERR_TIMEOUT";
    case ESP_FAIL: return "ESP_FAIL";
    default: return "ESP_ERR_OTHER";
  }
}

void canDriverPrintStatus(const char* label) {
  Serial.print("[TWAI] ");
  Serial.print(label);
  Serial.print(" ready:");
  Serial.print(canReady ? "YES" : "NO");
  Serial.print(" last:");
  Serial.print((int)lastCanError);
  Serial.print("(");
  Serial.print(canErrorToText(lastCanError));
  Serial.print(")");

  twai_status_info_t status;
  esp_err_t statusResult = twai_get_status_info(&status);
  if (statusResult != ESP_OK) {
    Serial.print(" status:");
    Serial.print((int)statusResult);
    Serial.print("(");
    Serial.print(canErrorToText(statusResult));
    Serial.println(")");
    return;
  }

  Serial.print(" state:");
  Serial.print(status.state);
  Serial.print(" txErr:");
  Serial.print(status.tx_error_counter);
  Serial.print(" rxErr:");
  Serial.print(status.rx_error_counter);
  Serial.print(" txQ:");
  Serial.print(status.msgs_to_tx);
  Serial.print(" rxQ:");
  Serial.print(status.msgs_to_rx);
  Serial.print(" txFail:");
  Serial.print(status.tx_failed_count);
  Serial.print(" rxMiss:");
  Serial.print(status.rx_missed_count);
  Serial.print(" busErr:");
  Serial.print(status.bus_error_count);
  Serial.print(" arbLost:");
  Serial.println(status.arb_lost_count);
}

static twai_message_t makeTwaiMessage(const CanFrame& frame) {
  twai_message_t message = {};
  message.identifier = frame.id & GDS_CAN_STANDARD_ID_MASK;
  message.extd = 0;
  message.rtr = 0;
  message.data_length_code = frame.dlc > GDS_CAN_DLC ? GDS_CAN_DLC : frame.dlc;

  for (uint8_t i = 0; i < message.data_length_code; i++) {
    message.data[i] = frame.data[i];
  }

  return message;
}

uint8_t canDriverBegin(uint8_t unusedPin) {
  (void)unusedPin;
  canReady = 0;
  lastCanError = ESP_OK;

  twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(
      (gpio_num_t)GDS_PIN_TWAI_TX,
      (gpio_num_t)GDS_PIN_TWAI_RX,
      TWAI_MODE_NO_ACK);
  twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  lastCanError = twai_driver_install(&generalConfig, &timingConfig, &filterConfig);
  if (lastCanError != ESP_OK) {
    return 0;
  }

  lastCanError = twai_start();
  if (lastCanError != ESP_OK) {
    twai_driver_uninstall();
    return 0;
  }

  lastCanError = ESP_OK;
  canReady = 1;
  return 1;
}

uint8_t canDriverSend(const CanFrame& frame) {
  if (!canReady || frame.dlc > GDS_CAN_DLC) {
    lastCanError = ESP_ERR_INVALID_STATE;
    return 0;
  }

  twai_message_t message = makeTwaiMessage(frame);
  lastCanError = twai_transmit(&message, pdMS_TO_TICKS(50));
  return lastCanError == ESP_OK;
}

uint8_t canDriverReceive(CanFrame& frame) {
  if (!canReady) {
    return 0;
  }

  twai_message_t message;
  if (twai_receive(&message, 0) != ESP_OK) {
    return 0;
  }

  if (message.extd || message.rtr) {
    return 0;
  }

  frame.id = (uint16_t)(message.identifier & GDS_CAN_STANDARD_ID_MASK);
  frame.dlc = message.data_length_code > GDS_CAN_DLC ? GDS_CAN_DLC : message.data_length_code;

  for (uint8_t i = 0; i < frame.dlc; i++) {
    frame.data[i] = message.data[i];
  }

  return 1;
}

uint8_t canDriverIsReady() {
  return canReady;
}