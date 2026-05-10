#ifndef HU_GDS_H
#define HU_GDS_H

#include <Arduino.h>

// ESP8266 Lolin D1 R2 Mini + MCP2515
// Adjust this pin if your MCP2515 CS wiring is different.
const uint8_t GDS_PIN_CAN_CS = D8;

const unsigned long GDS_SERIAL_BAUD = 115200;
const unsigned long GDS_HELP_INTERVAL_MS = 3000;

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

const uint8_t GDS_FAN_SPEED_MAX = 8;
const uint8_t GDS_TEMP_MIN = 18;
const uint8_t GDS_TEMP_MAX = 30;
const uint8_t GDS_VOLUME_MAX = 30;

const char GDS_WIFI_AP_SSID[] = "HVAC-HU";
const char GDS_WIFI_AP_PASSWORD[] = "12345678";
const uint16_t GDS_WEB_SERVER_PORT = 80;
const uint16_t GDS_WIFI_EEPROM_SIZE = 128;
const uint8_t GDS_WIFI_SSID_MAX_LEN = 32;
const uint8_t GDS_WIFI_PASSWORD_MAX_LEN = 64;

#endif
