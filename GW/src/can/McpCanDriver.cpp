#include "McpCanDriver.h"

#include <SPI.h>
#include <mcp_can.h>

static MCP_CAN mcpController(GDS_PIN_MCP_CS);
static uint8_t mcpReady = 0;

uint8_t mcpCanDriverBegin() {
  mcpReady = 0;
  if (!GDS_MCP_ENABLED) {
    Serial.println("MCP CAN:SKIP");
    return mcpReady;
  }

  // Setup interrupt pin
  pinMode(GDS_PIN_MCP_INT, INPUT_PULLUP);
  pinMode(GDS_PIN_MCP_CS, OUTPUT);
  digitalWrite(GDS_PIN_MCP_CS, HIGH);

  Serial.println("MCP2515 SPI init...");
  
  // Initialize SPI with lower frequency for stability
  SPI.begin(GDS_PIN_MCP_SCK, GDS_PIN_MCP_MISO, GDS_PIN_MCP_MOSI, GDS_PIN_MCP_CS);
  SPI.setFrequency(1000000);  // 1MHz for MCP2515 stability
  
  delay(100);

  byte result = mcpController.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ);
  if (result != CAN_OK) {
    Serial.print("MCP2515 begin failed: 0x");
    Serial.println(result, HEX);
    return 0;
  }

  // Set lower SPI frequency after initialization
  SPI.setFrequency(2000000);  // 2MHz for faster TX/RX
  
  mcpController.setMode(MCP_NORMAL);
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
  Serial.print(" SPI:5MHz");
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
      0,  // standard ID
      frame.dlc,
      (byte*)frame.data);

  if (result != CAN_OK) {
    // 0x7 = all TX buffers full - try to abort and retry once
    if (result == 0x7) {
      mcpController.abortTX();
      delayMicroseconds(100);
      
      result = mcpController.sendMsgBuf(
          frame.id & GDS_CAN_STANDARD_ID_MASK,
          0,
          frame.dlc,
          (byte*)frame.data);
      
      if (result != CAN_OK) {
        return 0;
      }
    } else {
      return 0;
    }
  }
  
  return 1;
}

uint8_t mcpCanDriverReceive(CanFrame& frame) {
  if (!mcpReady) {
    return 0;
  }

  // Check if data available
  uint8_t available = mcpController.checkReceive();
  if (available != CAN_MSGAVAIL) {
    return 0;
  }

  unsigned long rxId = 0;
  byte len = 0;
  byte buffer[GDS_CAN_DLC] = {};

  byte result = mcpController.readMsgBuf(&rxId, &len, buffer);
  if (result != CAN_OK) {
    Serial.print("DROP MCP read: 0x");
    Serial.println(result, HEX);
    return 0;
  }

  // Validate received data
  if (len == 0 || len > GDS_CAN_DLC) {
    Serial.print("DROP MCP DLC:");
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
  if (!GDS_MCP_ENABLED || !mcpReady) return;
}
