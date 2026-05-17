#include "CanDriver.h"

#include <driver/twai.h>

static uint8_t canReady = 0;

uint8_t canDriverBegin(uint8_t csPin) {
  (void)csPin;
  canReady = 0;

  twai_general_config_t generalConfig = TWAI_GENERAL_CONFIG_DEFAULT(
      (gpio_num_t)GDS_PIN_CAN_TX,
      (gpio_num_t)GDS_PIN_CAN_RX,
      TWAI_MODE_NORMAL);
  twai_timing_config_t timingConfig = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&generalConfig, &timingConfig, &filterConfig) != ESP_OK) {
    return 0;
  }

  if (twai_start() != ESP_OK) {
    twai_driver_uninstall();
    return 0;
  }

  canReady = 1;
  return 1;
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

  return twai_transmit(&message, pdMS_TO_TICKS(10)) == ESP_OK;
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
