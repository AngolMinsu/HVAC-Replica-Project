#ifndef TEST_MCP_CAN_H
#define TEST_MCP_CAN_H

#include <stdint.h>

#define MCP_ANY 0
#define CAN_500KBPS 0
#define MCP_16MHZ 0
#define MCP_NORMAL 0
#define CAN_OK 0
#define CAN_MSGAVAIL 1

class MCP_CAN {
 public:
  MCP_CAN(uint8_t csPin = 0) : csPin_(csPin), lastSendId(0), lastSendExt(0), lastSendLen(0), receiveAvailable(0), nextReceiveId(0), nextReceiveLen(0) {
    for (uint8_t i = 0; i < 8; i++) {
      lastSendData[i] = 0;
      nextReceiveData[i] = 0;
    }
  }

  uint8_t begin(uint8_t, uint8_t, uint8_t) {
    return CAN_OK;
  }

  uint8_t setMode(uint8_t) {
    return CAN_OK;
  }

  uint8_t sendMsgBuf(uint16_t id, uint8_t ext, uint8_t len, uint8_t* data) {
    lastSendId = id;
    lastSendExt = ext;
    lastSendLen = len;
    for (uint8_t i = 0; i < len && i < 8; i++) {
      lastSendData[i] = data[i];
    }
    return CAN_OK;
  }

  uint8_t checkReceive() {
    return receiveAvailable ? CAN_MSGAVAIL : 0;
  }

  uint8_t readMsgBuf(unsigned long* id, uint8_t* len, uint8_t* data) {
    *id = nextReceiveId;
    *len = nextReceiveLen;
    for (uint8_t i = 0; i < nextReceiveLen && i < 8; i++) {
      data[i] = nextReceiveData[i];
    }
    receiveAvailable = 0;
    return CAN_OK;
  }

  uint8_t csPin_;
  uint16_t lastSendId;
  uint8_t lastSendExt;
  uint8_t lastSendLen;
  uint8_t lastSendData[8];
  uint8_t receiveAvailable;
  unsigned long nextReceiveId;
  uint8_t nextReceiveLen;
  uint8_t nextReceiveData[8];
};

#endif
