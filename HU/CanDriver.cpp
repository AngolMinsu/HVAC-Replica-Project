#include "CanDriver.h"
#include <SPI.h>
#include <mcp_can.h>

static MCP_CAN canController(GDS_PIN_CAN_CS);
static uint8_t canReady = 0;

uint8_t canDriverBegin(uint8_t csPin) {
  (void)csPin;
  canReady = 0;

  if (canController.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) != CAN_OK) {
    return 0;
  }

  if (canController.setMode(MCP_NORMAL) != CAN_OK) {
    return 0;
  }

  canReady = 1;
  return 1;
}

uint8_t canDriverSend(const CanFrame& frame) {
  if (!canReady || frame.dlc > GDS_CAN_DLC) {
    return 0;
  }

  uint8_t data[GDS_CAN_DLC];
  for (uint8_t i = 0; i < frame.dlc; i++) {
    data[i] = frame.data[i];
  }

  return canController.sendMsgBuf(frame.id, 0, frame.dlc, data) == CAN_OK;
}

uint8_t canDriverReceive(CanFrame& frame) {
  if (!canReady || canController.checkReceive() != CAN_MSGAVAIL) {
    return 0;
  }

  unsigned long id = 0;
  uint8_t len = 0;
  uint8_t data[GDS_CAN_DLC];

  if (canController.readMsgBuf(&id, &len, data) != CAN_OK) {
    return 0;
  }

  frame.id = (uint16_t)(id & GDS_CAN_STANDARD_ID_MASK);
  frame.dlc = len > GDS_CAN_DLC ? GDS_CAN_DLC : len;
  for (uint8_t i = 0; i < frame.dlc; i++) {
    frame.data[i] = data[i];
  }

  return 1;
}

uint8_t canDriverIsReady() {
  return canReady;
}
