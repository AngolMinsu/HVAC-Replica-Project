#include "CanMonitor.h"

#include <Arduino.h>

void canMonitorPrintFrame(const char* label, const CanFrame& frame) {
  Serial.print("CAN ");
  Serial.print(label);
  Serial.print(" ID:0x");
  Serial.print(frame.id, HEX);
  Serial.print(" DLC:");
  Serial.print(frame.dlc);
  Serial.print(" DATA:");
  for (uint8_t i = 0; i < frame.dlc; i++) {
    Serial.print(' ');
    if (frame.data[i] < 0x10) Serial.print('0');
    Serial.print(frame.data[i], HEX);
  }
  Serial.println();
}
