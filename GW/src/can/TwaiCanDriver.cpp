#include "TwaiCanDriver.h"

#include <driver/twai.h>
#include <esp_err.h>

static uint8_t twaiReady = 0;
static uint32_t lastTwaiHealthLogMs = 0;

uint8_t twaiCanDriverBegin() {
  twaiReady = 0;
  if (!GDS_TWAI_ENABLED) {
    Serial.println("TWAI CAN:SKIP");
    return twaiReady;
  }

  twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(
      (gpio_num_t)GDS_PIN_TWAI_TX,
      (gpio_num_t)GDS_PIN_TWAI_RX,
      TWAI_MODE_NORMAL);
  generalConfig.tx_queue_len = 10;
  generalConfig.rx_queue_len = 20;

  twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  esp_err_t result = twai_driver_install(&generalConfig, &timingConfig, &filterConfig);
  if (result != ESP_OK) {
    Serial.print("TWAI install failed:");
    Serial.println(esp_err_to_name(result));
    return 0;
  }

  result = twai_start();
  if (result != ESP_OK) {
    Serial.print("TWAI start failed:");
    Serial.println(esp_err_to_name(result));
    twai_driver_uninstall();
    return 0;
  }

  twaiReady = 1;
  Serial.print("TWAI CAN:READY TX:");
  Serial.print(GDS_PIN_TWAI_TX);
  Serial.print(" RX:");
  Serial.println(GDS_PIN_TWAI_RX);
  return twaiReady;
}

uint8_t twaiCanDriverSend(const CanFrame& frame) {
  if (!twaiReady || frame.dlc > GDS_CAN_DLC) {
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
    Serial.print("TWAI TX failed:");
    Serial.println(esp_err_to_name(result));
    return 0;
  }
  return 1;
}

uint8_t twaiCanDriverReceive(CanFrame& frame) {
  if (!twaiReady) {
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

uint8_t twaiCanDriverIsReady() {
  return twaiReady;
}

void twaiCanDriverPollHealth() {
  if (!GDS_TWAI_ENABLED) return;

  twai_status_info_t status;
  esp_err_t result = twai_get_status_info(&status);
  if (result != ESP_OK) return;

  if (status.state == TWAI_STATE_BUS_OFF) {
    Serial.println("TWAI bus off. Recovery requested.");
    twai_initiate_recovery();
    twaiReady = 0;
    return;
  }

  if (status.state == TWAI_STATE_STOPPED && !twaiReady) {
    result = twai_start();
    twaiReady = (result == ESP_OK) ? 1 : 0;
    Serial.print("TWAI restart:");
    Serial.println(twaiReady ? "OK" : esp_err_to_name(result));
    return;
  }

  if (status.state == TWAI_STATE_RUNNING) {
    twaiReady = 1;
  }

  uint32_t now = millis();
  if (now - lastTwaiHealthLogMs >= 1000) {
    lastTwaiHealthLogMs = now;
    Serial.print("TWAI state:");
    Serial.print(status.state);
    Serial.print(" TXerr:");
    Serial.print(status.tx_error_counter);
    Serial.print(" RXerr:");
    Serial.print(status.rx_error_counter);
    Serial.print(" BusErr:");
    Serial.println(status.bus_error_count);
  }
}
