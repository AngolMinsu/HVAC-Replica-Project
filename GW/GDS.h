#ifndef GW_GDS_H
#define GW_GDS_H

#include <Arduino.h>

const unsigned long GDS_SERIAL_BAUD = 115200;
const uint16_t GDS_TASK_CAN_MS = 10;

// Keep forwarding off while checking which CAN side receives the bus traffic.
const uint8_t GDS_GW_FORWARD_ENABLED = 0;

// ESP32-S3 TWAI + SN65HVD230 side.
const uint8_t GDS_TWAI_ENABLED = 1;
const uint8_t GDS_PIN_TWAI_TX = 41;
const uint8_t GDS_PIN_TWAI_RX = 42;

// ESP32-S3 SPI + MCP2515 side.
const uint8_t GDS_MCP_ENABLED = 0;
const uint8_t GDS_PIN_MCP_INT = 14;
const uint8_t GDS_PIN_MCP_SCK = 18;
const uint8_t GDS_PIN_MCP_MOSI = 17;
const uint8_t GDS_PIN_MCP_MISO = 16;
const uint8_t GDS_PIN_MCP_CS = 15; 

const uint16_t GDS_CAN_ID_CONTROL_REQUEST = 0x100;
const uint16_t GDS_CAN_ID_CONTROL_RESPONSE = 0x101;
const uint16_t GDS_CAN_ID_HVAC_STATUS = 0x300;
const uint16_t GDS_CAN_STANDARD_ID_MASK = 0x7FF;
const uint8_t GDS_CAN_DLC = 8;

const uint8_t GDS_CAN_SERVICE_WRITE_REQUEST = 0x20;
const uint8_t GDS_CAN_SERVICE_WRITE_RESPONSE = 0x22;
const uint8_t GDS_CAN_SERVICE_READ_REQUEST = 0x30;
const uint8_t GDS_CAN_SERVICE_READ_RESPONSE = 0x32;

const uint8_t GDS_CAN_RESULT_NORMAL = 0x00;
const uint8_t GDS_CAN_RESULT_SUCCESS = 0x02;
const uint8_t GDS_CAN_RESULT_FAIL = 0x0F;

const uint8_t GDS_CAN_SIGNAL_POWER = 0x01;
const uint8_t GDS_CAN_SIGNAL_FAN_SPEED = 0x02;
const uint8_t GDS_CAN_SIGNAL_TEMPERATURE = 0x03;
const uint8_t GDS_CAN_SIGNAL_MODE = 0x04;
const uint8_t GDS_CAN_SIGNAL_AC = 0x05;
const uint8_t GDS_CAN_SIGNAL_AUTO = 0x06;
const uint8_t GDS_CAN_SIGNAL_INTAKE = 0x07;
const uint8_t GDS_CAN_SIGNAL_SCREEN_MODE = 0x08;
const uint8_t GDS_CAN_SIGNAL_MEDIA = 0x09;
const uint8_t GDS_CAN_SIGNAL_VOLUME = 0x0A;
const uint8_t GDS_CAN_SIGNAL_MAP = 0x0B;
const uint8_t GDS_CAN_SIGNAL_PASSENGER_TEMPERATURE = 0x0C;
const uint8_t GDS_CAN_SIGNAL_MUTE = 0x0D;
const uint8_t GDS_CAN_SIGNAL_HOME = 0x0E;
const uint8_t GDS_CAN_SIGNAL_MEDIA_MODE = 0x0F;
const uint8_t GDS_CAN_SIGNAL_MEDIA_INDEX = 0x10;
const uint8_t GDS_CAN_SIGNAL_DRIVER_ENCODER_SW = 0x11;
const uint8_t GDS_CAN_SIGNAL_PASSENGER_ENCODER_SW = 0x12;
const uint8_t GDS_CAN_SIGNAL_HU_FOCUS_PREV = 0x13;
const uint8_t GDS_CAN_SIGNAL_HU_FOCUS_NEXT = 0x14;
const uint8_t GDS_CAN_SIGNAL_HU_OPEN_HOME = 0x15;
const uint8_t GDS_CAN_SIGNAL_HU_OPEN_MAP = 0x16;
const uint8_t GDS_CAN_SIGNAL_HU_OPEN_MEDIA = 0x17;

#endif
