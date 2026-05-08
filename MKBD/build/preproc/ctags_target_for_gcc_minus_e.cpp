# 1 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino"
# 2 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 3 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2

# 5 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 6 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 7 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 8 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 9 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 10 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 11 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 12 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 13 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 14 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2

// =====================================================
// SH1106 OLED + DATC/INFO MODE DEMO
// Board : Arduino Uno R4
// OLED  : SH1106 I2C
// Fan   : 5V 2Pin DC Fan + 2N2222A Transistor
// =====================================================

// ===== OLED =====
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2((&u8g2_cb_r0));

// ===== 핀 맵 =====
#define BTN_2 GDS_PIN_BTN_2
#define BTN_3 GDS_PIN_BTN_3
#define BTN_4 GDS_PIN_BTN_4
#define BTN_5 GDS_PIN_BTN_5
#define BTN_SCREEN GDS_PIN_BTN_SCREEN

#define LED_STATUS GDS_PIN_LED_STATUS
#define FAN_MOTOR GDS_PIN_FAN_MOTOR
#define CAN_CS GDS_PIN_CAN_CS

// ===== 시스템 상태 =====
SystemState state;

// ===== 버튼 이전 상태 =====
ButtonHistory buttonHistory;

// ===== 디바운싱 =====
const unsigned long debounceDelay = GDS_DEBOUNCE_DELAY_MS;

// ===== 화면 갱신 =====
unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = GDS_DISPLAY_INTERVAL_MS;

uint8_t canTxCounter = 0;

ButtonLevels readButtonLevels();
uint8_t readButtonEvent();
int updateFanMotor();
uint8_t updateStatusLED();
uint8_t drawCurrentScreen();
uint8_t printSystemStatus(const SystemState& s);
uint8_t setupCan();
uint8_t processCanReceive();
uint8_t sendCanPayload(uint16_t id, const CanPayload& payload);
uint8_t broadcastHvacStatus(uint8_t signal);
uint8_t broadcastChangedHvacStatus(const SystemState& before, const SystemState& after);
uint8_t broadcastIfSignalChanged(const SystemState& before, const SystemState& after, uint8_t signal);

void setup() {
  pinMode(GDS_PIN_BTN_2, INPUT_PULLUP);
  pinMode(GDS_PIN_BTN_3, INPUT_PULLUP);
  pinMode(GDS_PIN_BTN_4, INPUT_PULLUP);
  pinMode(GDS_PIN_BTN_5, INPUT_PULLUP);
  pinMode(GDS_PIN_BTN_SCREEN, INPUT_PULLUP);

  pinMode(GDS_PIN_LED_STATUS, OUTPUT);
  pinMode(GDS_PIN_FAN_MOTOR, OUTPUT);

  digitalWrite(GDS_PIN_LED_STATUS, LOW);
  analogWrite(GDS_PIN_FAN_MOTOR, 0);

  SerialUSB.begin(9600);

  Wire.begin();
  Wire.setClock(100000);
  u8g2.begin();
  u8g2.setBusClock(100000);

  setupCan();

  initSystemState(state);
  initButtonHistory(buttonHistory);

  drawCurrentScreen();
  printSystemStatus(state);
}

void loop() {
  uint8_t button = readButtonEvent();
  if (button != APP_BUTTON_NONE) {
    SystemState before = state;
    uint8_t changed = handleButtonAction(state, button);
    if (changed) {
      printSystemStatus(state);
      broadcastChangedHvacStatus(before, state);
    }
  }

  if (processCanReceive()) {
    printSystemStatus(state);
  }

  // 화면이 INFO여도 공조 장치는 계속 동작해야 하므로 항상 실행
  updateStatusLED();
  updateFanMotor();

  if (shouldRefreshDisplay(millis(), lastDisplayTime, displayInterval)) {
    drawCurrentScreen();
    lastDisplayTime = millis();
  }
}

ButtonLevels readButtonLevels() {
  ButtonLevels levels;
  levels.btn2 = digitalRead(GDS_PIN_BTN_2);
  levels.btn3 = digitalRead(GDS_PIN_BTN_3);
  levels.btn4 = digitalRead(GDS_PIN_BTN_4);
  levels.btn5 = digitalRead(GDS_PIN_BTN_5);
  levels.screen = digitalRead(GDS_PIN_BTN_SCREEN);

  return levels;
}

uint8_t readButtonEvent() {
  ButtonLevels levels = readButtonLevels();
  return detectButtonEvent(buttonHistory, levels, millis(), debounceDelay);
}

int updateFanMotor() {
  int pwmValue = calculateFanPwm(state);

  analogWrite(GDS_PIN_FAN_MOTOR, pwmValue);

  return pwmValue;
}

uint8_t updateStatusLED() {
  uint8_t ledValue = calculateStatusLed(state);
  digitalWrite(GDS_PIN_LED_STATUS, ledValue);

  return ledValue;
}

uint8_t drawCurrentScreen() {
  if (state.screenMode == SCREEN_DATC) {
    datcDrawScreen(u8g2, state);
  } else {
    infoDrawScreen(u8g2, state);
  }

  return state.screenMode;
}

uint8_t printSystemStatus(const SystemState& s) {
  SerialUSB.print("SCREEN:");
  SerialUSB.print(screenModeToText(s.screenMode));
  SerialUSB.print(" | HVAC:");
  SerialUSB.print(hvacModeToText(s.hvacMode));
  SerialUSB.print(" | FAN:");
  SerialUSB.print(s.fanSpeed);
  SerialUSB.print(" | TEMP:");
  SerialUSB.print(s.setTemp);
  SerialUSB.print(" | AIR:");
  SerialUSB.print(windModeToText(s.windMode));
  SerialUSB.print(" | VOLUME:");
  SerialUSB.println(s.volume);

  return 1;
}

uint8_t setupCan() {
  uint8_t ready = canDriverBegin(GDS_PIN_CAN_CS);

  SerialUSB.print("CAN:");
  SerialUSB.println(ready ? "READY" : "FAIL");

  return ready;
}

uint8_t processCanReceive() {
  CanFrame rxFrame;
  if (!canDriverReceive(rxFrame)) {
    return 0;
  }

  canMonitorPrintFrame("RX", rxFrame);

  if (rxFrame.id != CAN_ID_CONTROL_REQUEST) {
    SerialUSB.println("CAN ERROR: UNKNOWN ID");
    return 0;
  }

  if (rxFrame.dlc != CAN_DLC) {
    SerialUSB.println("CAN ERROR: INVALID DLC");
    return 0;
  }

  CanPayload request = canPayloadFromBytes(rxFrame.data);
  CanPayload response;
  SystemState before = state;
  uint8_t changed = canProcessControlRequest(state, request, response);

  if (!sendCanPayload(CAN_ID_CONTROL_RESPONSE, response)) {
    SerialUSB.println("CAN ERROR: SEND FAIL");
  }

  if (changed) {
    broadcastChangedHvacStatus(before, state);
  }

  return changed;
}

uint8_t sendCanPayload(uint16_t id, const CanPayload& payload) {
  CanFrame frame;
  frame.id = id;
  frame.dlc = CAN_DLC;
  canPayloadToBytes(payload, frame.data);

  canMonitorPrintFrame("TX", frame);

  return canDriverSend(frame);
}

uint8_t broadcastHvacStatus(uint8_t signal) {
  CanPayload payload = canMakeStatusPayload(state, signal, canTxCounter++);

  return sendCanPayload(CAN_ID_HVAC_STATUS, payload);
}

uint8_t broadcastChangedHvacStatus(const SystemState& before, const SystemState& after) {
  uint8_t sentCount = 0;

  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_POWER);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_FAN_SPEED);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_TEMPERATURE);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_MODE);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_AC);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_AUTO);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_SCREEN_MODE);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_MEDIA);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_VOLUME);
  sentCount += broadcastIfSignalChanged(before, after, CAN_SIGNAL_MAP);

  return sentCount;
}

uint8_t broadcastIfSignalChanged(const SystemState& before, const SystemState& after, uint8_t signal) {
  uint8_t beforeValue = 0;
  uint8_t afterValue = 0;

  if (!canSignalValueFromState(before, signal, beforeValue) ||
      !canSignalValueFromState(after, signal, afterValue)) {
    return 0;
  }

  if (beforeValue == afterValue) {
    return 0;
  }

  return broadcastHvacStatus(signal);
}

# 270 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 271 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 272 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 273 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 274 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 275 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 276 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 277 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
# 278 "E:\\030_Portfolio\\010_Arduino\\040_CAN\\HVAC Replica\\main\\main.ino" 2
