#include "McpCanDriver.h"

#include <SPI.h>
#include <mcp_can.h>

static MCP_CAN mcpController(GDS_PIN_MCP_CS);
static uint8_t mcpReady = 0;

static const char* mcpResultToText(byte result) {
  switch (result) {
    case CAN_OK: return "CAN_OK";
    case CAN_FAILINIT: return "CAN_FAILINIT";
    case CAN_FAILTX: return "CAN_FAILTX";
    case CAN_MSGAVAIL: return "CAN_MSGAVAIL";
    case CAN_NOMSG: return "CAN_NOMSG";
    case CAN_CTRLERROR: return "CAN_CTRLERROR";
    case CAN_GETTXBFTIMEOUT: return "CAN_GETTXBFTIMEOUT";
    case CAN_SENDMSGTIMEOUT: return "CAN_SENDMSGTIMEOUT";
    case CAN_FAIL: return "CAN_FAIL";
    default: return "UNKNOWN";
  }
}

static void printMcpErrorDetail(const char* label, byte result) {
  Serial.print(label);
  Serial.print(" result:");
  Serial.print(result);
  Serial.print("(");
  Serial.print(mcpResultToText(result));
  Serial.print(") err:0x");
  Serial.print(mcpController.getError(), HEX);
  Serial.print(" rxErr:");
  Serial.print(mcpController.errorCountRX());
  Serial.print(" txErr:");
  Serial.print(mcpController.errorCountTX());
  Serial.print(" INT:");
  Serial.println(digitalRead(GDS_PIN_MCP_INT));
}

uint8_t mcpCanDriverBegin() {
  mcpReady = 0;
  if (!GDS_MCP_ENABLED) {
    Serial.println("MCP CAN:SKIP");
    return mcpReady;
  }

  pinMode(GDS_PIN_MCP_INT, INPUT_PULLUP);
  pinMode(GDS_PIN_MCP_CS, OUTPUT);
  digitalWrite(GDS_PIN_MCP_CS, HIGH);

  Serial.println("MCP2515 SPI init...");
  SPI.begin(GDS_PIN_MCP_SCK, GDS_PIN_MCP_MISO, GDS_PIN_MCP_MOSI, GDS_PIN_MCP_CS);
  SPI.setFrequency(1000000);
  delay(100);

  byte result = mcpController.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);
  if (result != CAN_OK) {
    printMcpErrorDetail("MCP2515 begin failed", result);
    return 0;
  }

  SPI.setFrequency(2000000);

  result = mcpController.setMode(MCP_NORMAL);
  if (result != CAN_OK) {
    printMcpErrorDetail("MCP2515 normal mode failed", result);
    return 0;
  }

  delay(50);
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
  Serial.print(GDS_PIN_MCP_MISO);
  Serial.print(" SPI:2MHz");
  Serial.println();
  return mcpReady;
}

uint8_t mcpCanDriverSend(const CanFrame& frame) {
  if (!mcpReady) {
    Serial.println("MCP TX: driver not ready");
    return 0;
  }

  if (frame.dlc == 0 || frame.dlc > GDS_CAN_DLC) {
    Serial.print("MCP TX: invalid DLC:");
    Serial.println(frame.dlc);
    return 0;
  }

  byte result = mcpController.sendMsgBuf(
      frame.id & GDS_CAN_STANDARD_ID_MASK,
      0,
      frame.dlc,
      (byte*)frame.data);

  if (result == CAN_OK) {
    return 1;
  }

  printMcpErrorDetail("MCP TX first failed", result);

  if (result == CAN_GETTXBFTIMEOUT || result == CAN_SENDMSGTIMEOUT) {
    mcpController.abortTX();
    delayMicroseconds(100);

    result = mcpController.sendMsgBuf(
        frame.id & GDS_CAN_STANDARD_ID_MASK,
        0,
        frame.dlc,
        (byte*)frame.data);

    if (result == CAN_OK) {
      Serial.println("MCP TX retry OK");
      return 1;
    }

    printMcpErrorDetail("MCP TX retry failed", result);
  }

  return 0;
}

uint8_t mcpCanDriverReceive(CanFrame& frame) {
  if (!mcpReady) {
    return 0;
  }

  uint8_t available = mcpController.checkReceive();
  if (available != CAN_MSGAVAIL) {
    return 0;
  }

  unsigned long rxId = 0;
  byte len = 0;
  byte buffer[GDS_CAN_DLC] = {};

  byte result = mcpController.readMsgBuf(&rxId, &len, buffer);
  if (result != CAN_OK) {
    printMcpErrorDetail("MCP RX read failed", result);
    return 0;
  }

  if (len == 0 || len > GDS_CAN_DLC) {
    Serial.print("MCP RX DROP DLC:");
    Serial.println(len);
    return 0;
  }

  frame.id = (uint16_t)(rxId & GDS_CAN_STANDARD_ID_MASK);
  frame.dlc = len;
  for (uint8_t i = 0; i < frame.dlc; i++) {
    frame.data[i] = buffer[i];
  }
  return 1;
}

uint8_t mcpCanDriverIsReady() {
  return mcpReady;
}

void mcpCanDriverPollHealth() {
}