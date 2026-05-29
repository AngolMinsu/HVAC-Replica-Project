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

void canMonitorPrintPayloadSummary(const CanPayload& payload) {
  Serial.print("  Service:0x");
  if (payload.service < 0x10) Serial.print('0');
  Serial.print(payload.service, HEX);
  Serial.print(" Result:0x");
  if (payload.result < 0x10) Serial.print('0');
  Serial.print(payload.result, HEX);
  Serial.print(" Signal:");
  Serial.print(canSignalToText(payload.signal));
  Serial.print(" Value:");
  Serial.print(payload.value);
  Serial.print(" Counter:");
  Serial.print(payload.counter);
  Serial.print(" Checksum:");
  Serial.println(canValidateChecksum(payload) ? "OK" : "NOK");
}
