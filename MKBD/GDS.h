#ifndef GDS_H
#define GDS_H

#include <Arduino.h>

// Hardware pins
const uint8_t GDS_PIN_DRIVER_ENC_A = 2;
const uint8_t GDS_PIN_DRIVER_ENC_B = 3;
const uint8_t GDS_PIN_DRIVER_ENC_SW = 4;
const uint8_t GDS_PIN_BTN_FAN_UP = 5;
const uint8_t GDS_PIN_BTN_FAN_DOWN = 6;
const uint8_t GDS_PIN_BTN_SCREEN = 7;
const uint8_t GDS_PIN_FAN_MOTOR = 9;
const uint8_t GDS_PIN_CAN_CS = 10;
const uint8_t GDS_PIN_PASSENGER_ENC_A = A0;
const uint8_t GDS_PIN_PASSENGER_ENC_B = A1;
const uint8_t GDS_PIN_PASSENGER_ENC_SW = A2;
const uint8_t GDS_PIN_BTN_WIND_RADIO = A3;

// Timing
const unsigned long GDS_DEBOUNCE_DELAY_MS = 60;
const unsigned long GDS_DISPLAY_INTERVAL_MS = 100;

// HVAC / INFO state ranges
const uint8_t GDS_FAN_SPEED_MIN = 0;
const uint8_t GDS_FAN_SPEED_MAX = 8;
const uint8_t GDS_TEMP_MIN = 18;
const uint8_t GDS_TEMP_DEFAULT = 24;
const uint8_t GDS_TEMP_MAX = 30;
const uint8_t GDS_VOLUME_MIN = 0;
const uint8_t GDS_VOLUME_DEFAULT = 10;
const uint8_t GDS_VOLUME_MAX = 30;
const uint8_t GDS_RADIO_TUNE_MIN = 0;
const uint8_t GDS_RADIO_TUNE_DEFAULT = 10;
const uint8_t GDS_RADIO_TUNE_MAX = 30;

// Fan PWM policy
const int GDS_FAN_PWM_OFF = 0;
const int GDS_FAN_PWM_MIN = 90;
const int GDS_FAN_PWM_MAX = 255;

// CAN frame configuration
const uint16_t GDS_CAN_ID_CONTROL_REQUEST = 0x100;
const uint16_t GDS_CAN_ID_CONTROL_RESPONSE = 0x101;
const uint16_t GDS_CAN_ID_HVAC_STATUS = 0x300;
const uint16_t GDS_CAN_STANDARD_ID_MASK = 0x7FF;
const uint8_t GDS_CAN_DLC = 8;

// CAN service values
const uint8_t GDS_CAN_SERVICE_WRITE_REQUEST = 0x20;
const uint8_t GDS_CAN_SERVICE_WRITE_RESPONSE = 0x22;
const uint8_t GDS_CAN_SERVICE_READ_REQUEST = 0x30;
const uint8_t GDS_CAN_SERVICE_READ_RESPONSE = 0x32;

// CAN result values
const uint8_t GDS_CAN_RESULT_NORMAL = 0x00;
const uint8_t GDS_CAN_RESULT_SUCCESS = 0x02;
const uint8_t GDS_CAN_RESULT_FAIL = 0x0F;

// CAN signal values
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

// CAN error values for payload Option byte on failure responses
const uint8_t GDS_CAN_ERROR_NONE = 0x00;
const uint8_t GDS_CAN_ERROR_CHECKSUM = 0x01;
const uint8_t GDS_CAN_ERROR_UNSUPPORTED_SERVICE = 0x02;
const uint8_t GDS_CAN_ERROR_UNSUPPORTED_SIGNAL = 0x03;
const uint8_t GDS_CAN_ERROR_VALUE_OUT_OF_RANGE = 0x04;
const uint8_t GDS_CAN_ERROR_INVALID_DLC = 0x05;
const uint8_t GDS_CAN_ERROR_COUNTER = 0x06;
const uint8_t GDS_CAN_ERROR_DRIVER = 0x07;

#endif
