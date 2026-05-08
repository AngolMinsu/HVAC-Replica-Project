#include "CanMonitor.h"

static void printHexByte(uint8_t value) {
  if (value < 0x10) {
    Serial.print("0");
  }

  Serial.print(value, HEX);
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
}
