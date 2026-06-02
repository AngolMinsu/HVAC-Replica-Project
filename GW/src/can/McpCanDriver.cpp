#include "McpCanDriver.h"

#include <SPI.h>
#include <mcp_can.h>

static MCP_CAN mcpController(GDS_PIN_MCP_CS);
static uint8_t mcpReady = 0;
static uint32_t lastMcpHealthLogMs = 0;

uint8_t mcpCanDriverBegin() {
  mcpReady = 0;
  if (!GDS_MCP_ENABLED) {
    Serial.println("MCP CAN:SKIP");
    return mcpReady;
  }

  pinMode(GDS_PIN_MCP_INT, INPUT_PULLUP);
  pinMode(GDS_PIN_MCP_CS, OUTPUT);
  digitalWrite(GDS_PIN_MCP_CS, HIGH);

  SPI.begin(GDS_PIN_MCP_SCK, GDS_PIN_MCP_MISO, GDS_PIN_MCP_MOSI, GDS_PIN_MCP_CS);

  byte result = mcpController.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);
  if (result != CAN_OK) {
    Serial.print("MCP2515 begin failed:");
    Serial.println(result);
    Serial.println("Check wiring. Try MCP_16MHZ if the module uses 16MHz oscillator.");
    return 0;
  }

  mcpController.setMode(MCP_NORMAL);
  mcpReady = 1;

  Serial.print("MCP CAN:READY INT:");
  Serial.print(GDS_PIN_MCP_INT);
  Serial.print(" CS:");
  Serial.print(GDS_PIN_MCP_CS);
  Serial.print(" SCK:");
  Serial.print(GDS_PIN_MCP_SCK);
  Serial.print(" MOSI:");
  Serial.print(GDS_PIN_MCP_MOSI);
  Serial.print(" MISO:");
  Serial.println(GDS_PIN_MCP_MISO);
  return mcpReady;
}

uint8_t mcpCanDriverSend(const CanFrame& frame) {
  if (!mcpReady || frame.dlc > GDS_CAN_DLC) {
    return 0;
  }

  byte result = mcpController.sendMsgBuf(
      frame.id & GDS_CAN_STANDARD_ID_MASK,
      0,
      frame.dlc,
      (byte*)frame.data);

  if (result != CAN_OK) {
    Serial.print("MCP TX failed:");
    Serial.println(result);
    return 0;
  }
  return 1;
}

uint8_t mcpCanDriverReceive(CanFrame& frame) {
  if (!mcpReady) {
    return 0;
  }

  if (mcpController.checkReceive() != CAN_MSGAVAIL) {
    return 0;
  }

  unsigned long rxId = 0;
  byte len = 0;
  byte buffer[GDS_CAN_DLC] = {};

  byte result = mcpController.readMsgBuf(&rxId, &len, buffer);
  if (result != CAN_OK) {
    Serial.print("MCP RX read failed:");
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

uint8_t mcpCanDriverIsReady() {
  return mcpReady;
}

void mcpCanDriverPollHealth() {
  if (!GDS_MCP_ENABLED) return;

  uint32_t now = millis();
  if (now - lastMcpHealthLogMs >= 1000) {
    lastMcpHealthLogMs = now;
    uint8_t hasError = mcpController.checkError();
    uint8_t errorFlag = mcpController.getError();
    uint8_t rxErrorCount = mcpController.errorCountRX();
    uint8_t txErrorCount = mcpController.errorCountTX();

    Serial.print("MCP CAN ready:");
    Serial.print(mcpReady ? "YES" : "NO");
    Serial.print(" INT:");
    Serial.print(digitalRead(GDS_PIN_MCP_INT));
    Serial.print(" err:");
    Serial.print(hasError);
    Serial.print(" flag:0x");
    Serial.print(errorFlag, HEX);
    Serial.print(" rxErr:");
    Serial.print(rxErrorCount);
    Serial.print(" txErr:");
    Serial.println(txErrorCount);
  }
}
