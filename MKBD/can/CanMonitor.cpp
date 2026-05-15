#include "CanMonitor.h"
#include "CanProtocol.h"

static void printHexByte(uint8_t value) {
  if (value < 0x10) {
    Serial.print("0");
  }

  Serial.print(value, HEX);
}

static void printPayloadSummary(const CanPayload& payload) {
  Serial.print("  Service:0x");
  printHexByte(payload.service);
  Serial.print(" Result:0x");
  printHexByte(payload.result);
  Serial.print(" Signal:0x");
  printHexByte(payload.signal);
  Serial.print(" Value:");
  Serial.print(payload.value);
  Serial.print(" Option:0x");
  printHexByte(payload.option);
  Serial.print(" Counter:");
  Serial.print(payload.counter);
  Serial.print(" Checksum:");
  Serial.println(canValidateChecksum(payload) ? "OK" : "NOK");
}

void canMonitorPrintFrame(const char* direction, const CanFrame& frame) {
  Serial.print("CAN ");
  Serial.print(direction);
  Serial.print(" ID:0x");
  Serial.print(frame.id, HEX);
  Serial.print(" DLC:");
  Serial.print(frame.dlc);
  Serial.print(" DATA:");

  for (uint8_t i = 0; i < frame.dlc; i++) {
    Serial.print(" ");
    printHexByte(frame.data[i]);
  }

  Serial.println();

  if (frame.dlc == CAN_DLC) {
    CanPayload payload = canPayloadFromBytes(frame.data);
    printPayloadSummary(payload);
  }
}
