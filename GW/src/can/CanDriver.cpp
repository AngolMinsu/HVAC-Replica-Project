#include "CanDriver.h"

#include <SPI.h>
#include <mcp_can.h>

static MCP_CAN canController(GDS_PIN_CAN_CS);
static uint8_t canReady = 0;
static uint32_t lastHealthLogMs = 0;

uint8_t canDriverBegin() {
  canReady = 0;
  if (!GDS_CAN_ENABLED) {
    Serial.println("CAN:SKIP");
    return canReady;
  }

  pinMode(GDS_PIN_CAN_INT, INPUT_PULLUP);
  pinMode(GDS_PIN_CAN_CS, OUTPUT);
  digitalWrite(GDS_PIN_CAN_CS, HIGH);

  SPI.begin(GDS_PIN_CAN_SCK, GDS_PIN_CAN_MISO, GDS_PIN_CAN_MOSI, GDS_PIN_CAN_CS);

  byte result = canController.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);
  if (result != CAN_OK) {
    Serial.print("MCP2515 begin failed:");
    Serial.println(result);
    Serial.println("Check CS/SCK/MOSI/MISO wiring and MCP2515 oscillator. Try MCP_16MHZ if needed.");
    return 0;
  }

  canController.setMode(MCP_NORMAL);
  canReady = 1;

  Serial.print("CAN:MCP2515 READY INT:");
  Serial.print(GDS_PIN_CAN_INT);
  Serial.print(" CS:");
  Serial.print(GDS_PIN_CAN_CS);
  Serial.print(" SCK:");
  Serial.print(GDS_PIN_CAN_SCK);
  Serial.print(" MOSI:");
  Serial.print(GDS_PIN_CAN_MOSI);
  Serial.print(" MISO:");
  Serial.println(GDS_PIN_CAN_MISO);
  return canReady;
}

uint8_t canDriverSend(const CanFrame& frame) {
  if (!canReady || frame.dlc > GDS_CAN_DLC) {
    return 0;
  }

  byte result = canController.sendMsgBuf(
      frame.id & GDS_CAN_STANDARD_ID_MASK,
      0,
      frame.dlc,
      (byte*)frame.data);

  if (result != CAN_OK) {
    Serial.print("MCP2515 transmit failed:");
    Serial.println(result);
    return 0;
  }
  return 1;
}

uint8_t canDriverReceive(CanFrame& frame) {
  if (!canReady) {
    return 0;
  }

  if (canController.checkReceive() != CAN_MSGAVAIL) {
    return 0;
  }

  unsigned long rxId = 0;
  byte len = 0;
  byte buffer[GDS_CAN_DLC] = {};

  byte result = canController.readMsgBuf(&rxId, &len, buffer);
  if (result != CAN_OK) {
    Serial.print("MCP2515 read failed:");
    Serial.println(result);
    return 0;
  }

  frame.id = (uint16_t)(rxId & GDS_CAN_STANDARD_ID_MASK);
  frame.dlc = len > GDS_CAN_DLC ? GDS_CAN_DLC : len;
  for (uint8_t i = 0; i < frame.dlc; i++) {
    frame.data[i] = buffer[i];
  }
  return 1;
}

uint8_t canDriverIsReady() {
  return canReady;
}

void canDriverPollHealth() {
  if (!GDS_CAN_ENABLED) return;

  uint32_t now = millis();
  if (now - lastHealthLogMs >= 1000) {
    lastHealthLogMs = now;
    Serial.print("CAN MCP ready:");
    Serial.print(canReady ? "YES" : "NO");
    Serial.print(" INT:");
    Serial.println(digitalRead(GDS_PIN_CAN_INT));
  }
}
