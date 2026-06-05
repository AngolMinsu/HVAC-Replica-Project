#include "CanDriver.h"

#include <Arduino.h>
#include <driver/twai.h>
#include <esp_err.h>

#include "../vendor/waveshare_7b/io_extension.h"

static uint8_t canReady = 0;

uint8_t canDriverBegin() {
  canReady = 0;
  if (!GDS_CAN_ENABLED) {
    Serial.println("CAN:SKIP");
    return canReady;
  }

  IO_EXTENSION_Output(GDS_IO_CAN_SELECT, 1);
  Serial.print("CAN select IO");
  Serial.print(GDS_IO_CAN_SELECT);
  Serial.println(":CAN");

  twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(
      (gpio_num_t)GDS_PIN_CAN_TX,
      (gpio_num_t)GDS_PIN_CAN_RX,
      TWAI_MODE_NORMAL);
  generalConfig.tx_queue_len = 10;
  generalConfig.rx_queue_len = 20;

  twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  esp_err_t result = twai_driver_install(&generalConfig, &timingConfig, &filterConfig);
  if (result != ESP_OK) {
    Serial.print("CAN driver install failed:");
    Serial.println(esp_err_to_name(result));
    return 0;
  }

  result = twai_start();
  if (result != ESP_OK) {
    Serial.print("CAN driver start failed:");
    Serial.println(esp_err_to_name(result));
    twai_driver_uninstall();
    return 0;
  }

  canReady = 1;
  Serial.print("CAN:READY TX:");
  Serial.print(GDS_PIN_CAN_TX);
  Serial.print(" RX:");
  Serial.println(GDS_PIN_CAN_RX);
  return canReady;
}

uint8_t canDriverSend(const CanFrame& frame) {
  if (!canReady || frame.dlc > GDS_CAN_DLC) {
    return 0;
  }

  twai_message_t message = {};
  message.identifier = frame.id & GDS_CAN_STANDARD_ID_MASK;
  message.extd = 0;
  message.rtr = 0;
  message.data_length_code = frame.dlc;

  for (uint8_t i = 0; i < frame.dlc; i++) {
    message.data[i] = frame.data[i];
  }

  esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(10));
  if (result != ESP_OK) {
    Serial.print("CAN transmit failed:");
    Serial.println(esp_err_to_name(result));
    return 0;
  }

  return 1;
}

uint8_t canDriverReceive(CanFrame& frame) {
  if (!canReady) {
    return 0;
  }

  twai_message_t message = {};
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

void canDriverPollHealth() {
  if (!GDS_CAN_ENABLED) {
    return;
  }

  twai_status_info_t status;
  esp_err_t result = twai_get_status_info(&status);
  if (result != ESP_OK) {
    return;
  }

  if (status.state == TWAI_STATE_BUS_OFF) {
    Serial.println("CAN bus off. Recovery requested.");
    twai_initiate_recovery();
    canReady = 0;
    return;
  }

  if (status.state == TWAI_STATE_STOPPED && !canReady) {
    result = twai_start();
    canReady = (result == ESP_OK) ? 1 : 0;
    Serial.print("CAN restart:");
    Serial.println(canReady ? "OK" : esp_err_to_name(result));
    return;
  }

  if (status.state == TWAI_STATE_RUNNING) {
    canReady = 1;
  }

}
