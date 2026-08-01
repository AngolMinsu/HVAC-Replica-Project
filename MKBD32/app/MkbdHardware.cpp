#include "MkbdHardware.h"

#include <U8g2lib.h>
#include <Wire.h>

#include "../GDS.h"
#include "../button/ButtonInput.h"
#include "../display/Datc.h"
#include "../display/Info.h"
#include "../encoder/EncoderInput.h"
#include "AppLogic.h"

static U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0);
static ButtonHistory buttonHistory;
static EncoderHistory encoderHistory;

static void setupInputPins() {
  pinMode(GDS_PIN_DRIVER_ENC_A, INPUT_PULLUP);
  pinMode(GDS_PIN_DRIVER_ENC_B, INPUT_PULLUP);
  pinMode(GDS_PIN_DRIVER_ENC_SW, INPUT_PULLUP);
  pinMode(GDS_PIN_PASSENGER_ENC_A, INPUT_PULLUP);
  pinMode(GDS_PIN_PASSENGER_ENC_B, INPUT_PULLUP);
  pinMode(GDS_PIN_PASSENGER_ENC_SW, INPUT_PULLUP);
  pinMode(GDS_PIN_BTN_FAN_UP, INPUT_PULLUP);
  pinMode(GDS_PIN_BTN_FAN_DOWN, INPUT_PULLUP);
  pinMode(GDS_PIN_BTN_SCREEN, INPUT_PULLUP);
  pinMode(GDS_PIN_BTN_WIND_MEDIA, INPUT_PULLUP);
}

static ButtonLevels readButtonLevels() {
  ButtonLevels levels;
  levels.fanUp = digitalRead(GDS_PIN_BTN_FAN_UP);
  levels.fanDown = digitalRead(GDS_PIN_BTN_FAN_DOWN);
  levels.screen = digitalRead(GDS_PIN_BTN_SCREEN);
  levels.windMedia = digitalRead(GDS_PIN_BTN_WIND_MEDIA);
  return levels;
}

static EncoderLevels readEncoderLevels() {
  EncoderLevels levels;
  levels.driverA = digitalRead(GDS_PIN_DRIVER_ENC_A);
  levels.driverB = digitalRead(GDS_PIN_DRIVER_ENC_B);
  levels.driverSw = digitalRead(GDS_PIN_DRIVER_ENC_SW);
  levels.passengerA = digitalRead(GDS_PIN_PASSENGER_ENC_A);
  levels.passengerB = digitalRead(GDS_PIN_PASSENGER_ENC_B);
  levels.passengerSw = digitalRead(GDS_PIN_PASSENGER_ENC_SW);
  return levels;
}

void mkbdHardwareBegin() {
  setupInputPins();
  pinMode(GDS_PIN_FAN_MOTOR, OUTPUT);
  analogWrite(GDS_PIN_FAN_MOTOR, GDS_FAN_PWM_OFF);

  Wire.begin(GDS_PIN_OLED_SDA, GDS_PIN_OLED_SCL);
  Wire.setClock(100000);
  display.begin();
  display.setBusClock(100000);

  initButtonHistory(buttonHistory);
  initEncoderHistory(encoderHistory);
}

uint8_t mkbdHardwareReadButtonEvent() {
  return detectButtonEvent(buttonHistory, readButtonLevels(), millis(), GDS_DEBOUNCE_DELAY_MS);
}

uint8_t mkbdHardwareReadEncoderEvent() {
  return detectEncoderEvent(encoderHistory, readEncoderLevels(), millis(), GDS_ENCODER_DEBOUNCE_DELAY_MS);
}

int mkbdHardwareUpdateFan(const SystemState& state) {
  int pwmValue = calculateFanPwm(state);
  analogWrite(GDS_PIN_FAN_MOTOR, pwmValue);
  return pwmValue;
}

uint8_t mkbdHardwareDrawDisplay(const SystemState& state) {
  if (state.screenMode == SCREEN_DATC) {
    datcDrawScreen(display, state);
  } else {
    infoDrawScreen(display, state);
  }
  return state.screenMode;
}
