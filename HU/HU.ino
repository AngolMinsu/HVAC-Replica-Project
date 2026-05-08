#include <Arduino.h>
#include <SPI.h>

#include "GDS.h"
#include "CanDriver.h"
#include "CanMonitor.h"
#include "CanProtocol.h"
#include "CommandParser.h"

uint8_t huTxCounter = 0;
unsigned long lastHelpTime = 0;

void processSerialCommand();
uint8_t sendRequest(uint8_t service, uint8_t signal, uint8_t value);
uint8_t sendCanPayload(uint16_t id, const CanPayload& payload);
void processCanReceive();

void setup() {
  Serial.begin(GDS_SERIAL_BAUD);
  delay(500);

  Serial.println();
  Serial.println("ESP8266 Head Unit CAN Test Node");

  uint8_t ready = canDriverBegin(GDS_PIN_CAN_CS);
  Serial.print("CAN:");
  Serial.println(ready ? "READY" : "FAIL");

  printHuHelp();
  lastHelpTime = millis();
}

void loop() {
  processSerialCommand();
  processCanReceive();

  if (millis() - lastHelpTime > GDS_HELP_INTERVAL_MS && !canDriverIsReady()) {
    Serial.println("CAN is not ready. Check MCP2515 wiring, CS pin, and oscillator.");
    lastHelpTime = millis();
  }
}

void processSerialCommand() {
  if (!Serial.available()) {
    return;
  }

  String line = Serial.readStringUntil('\n');
  line.trim();

  if (line.length() == 0) {
    return;
  }

  HuCommand command = parseHuCommand(line);
  if (!command.valid) {
    Serial.println("Invalid command. Type 'help'.");
    return;
  }

  if (!sendRequest(command.service, command.signal, command.value)) {
    Serial.println("CAN ERROR: SEND FAIL");
  }
}

uint8_t sendRequest(uint8_t service, uint8_t signal, uint8_t value) {
  CanPayload payload = canMakePayload(service, CAN_RESULT_NORMAL, signal, value, 0, huTxCounter++);
  return sendCanPayload(CAN_ID_CONTROL_REQUEST, payload);
}

uint8_t sendCanPayload(uint16_t id, const CanPayload& payload) {
  CanFrame frame;
  frame.id = id;
  frame.dlc = CAN_DLC;
  canPayloadToBytes(payload, frame.data);

  canMonitorPrintFrame("TX", frame);
  canMonitorPrintPayloadSummary(payload);

  return canDriverSend(frame);
}

void processCanReceive() {
  CanFrame frame;
  if (!canDriverReceive(frame)) {
    return;
  }

  canMonitorPrintFrame("RX", frame);

  if (frame.dlc != CAN_DLC) {
    Serial.println("  CAN ERROR: INVALID DLC");
    return;
  }

  if (frame.id != CAN_ID_CONTROL_RESPONSE && frame.id != CAN_ID_HVAC_STATUS) {
    Serial.println("  CAN NOTICE: IGNORED ID");
    return;
  }

  CanPayload payload = canPayloadFromBytes(frame.data);
  canMonitorPrintPayloadSummary(payload);
}
