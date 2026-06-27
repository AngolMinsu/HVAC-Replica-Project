#include "CanTask.h"

#include "../../../../GDS.h"
#include "../../../can/CanProtocol.h"
#include "../../../can/McpCanDriver.h"
#include "../../../can/TwaiCanDriver.h"
#include "../../../gateway/GatewayRouter.h"

static uint32_t huToMkbdCount = 0;
static uint32_t mkbdToHuCount = 0;

static void printHexByte(uint8_t value) {
  if (value < 0x10) Serial.print("0");
  Serial.print(value, HEX);
}

static void printFrameData(const CanFrame& frame) {
  Serial.print(" DATA:");
  for (uint8_t i = 0; i < frame.dlc; i++) {
    Serial.print(" ");
    printHexByte(frame.data[i]);
  }
}

static const char* serviceToText(uint8_t service) {
  switch (service) {
    case GDS_CAN_SERVICE_WRITE_REQUEST: return "WRITE_REQ";
    case GDS_CAN_SERVICE_WRITE_RESPONSE: return "WRITE_RESP";
    case GDS_CAN_SERVICE_READ_REQUEST: return "READ_REQ";
    case GDS_CAN_SERVICE_READ_RESPONSE: return "READ_RESP";
    default: return "UNKNOWN_SERVICE";
  }
}

static const char* resultToText(uint8_t result) {
  switch (result) {
    case GDS_CAN_RESULT_NORMAL: return "NORMAL";
    case GDS_CAN_RESULT_SUCCESS: return "SUCCESS";
    case GDS_CAN_RESULT_FAIL: return "FAIL";
    default: return "UNKNOWN_RESULT";
  }
}

static void printPayloadMeaning(const CanFrame& frame) {
  if (frame.dlc != GDS_CAN_DLC) {
    Serial.print(" Meaning:DLC_INVALID");
    return;
  }

  CanPayload payload = canPayloadFromBytes(frame.data);

  Serial.print(" Meaning:");
  Serial.print(serviceToText(payload.service));
  Serial.print("/");
  Serial.print(resultToText(payload.result));
  Serial.print(" Signal:");
  Serial.print(canSignalToText(payload.signal));
  Serial.print(" Value:");
  Serial.print(payload.value);
  Serial.print(" Option:0x");
  printHexByte(payload.option);
  Serial.print(" Count:");
  Serial.print(payload.counter);
  Serial.print(" Checksum:");
  Serial.print(canValidateChecksum(payload) ? "OK" : "NOK");
}

static void printRouteResult(const char* direction, uint32_t count, const CanFrame& frame, uint8_t sent) {
  Serial.print("GW ");
  Serial.print(direction);
  Serial.print(" #");
  Serial.print(count);
  Serial.print(" ID:0x");
  Serial.print(frame.id, HEX);
  Serial.print(" DLC:");
  Serial.print(frame.dlc);
  printFrameData(frame);
  printPayloadMeaning(frame);
  Serial.print(" Route:");
  Serial.println(sent ? "OK" : "FAIL");
}

static void routeHuToMkbd() {
  CanFrame rxFrame = {};
  while (twaiCanDriverReceive(rxFrame)) {
    CanFrame txFrame = {};
    if (!gatewayRouteFrame("HU->MKBD", rxFrame, txFrame)) {
      continue;
    }

    uint8_t sent = GDS_GW_FORWARD_ENABLED ? mcpCanDriverSend(txFrame) : 0;
    printRouteResult("HU->MKBD", ++huToMkbdCount, rxFrame, sent);
  }
}

static void routeMkbdToHu() {
  CanFrame rxFrame = {};
  while (mcpCanDriverReceive(rxFrame)) {
    CanFrame txFrame = {};
    if (!gatewayRouteFrame("MKBD->HU", rxFrame, txFrame)) {
      continue;
    }

    uint8_t sent = GDS_GW_FORWARD_ENABLED ? twaiCanDriverSend(txFrame) : 0;
    printRouteResult("MKBD->HU", ++mkbdToHuCount, rxFrame, sent);
  }
}

void gatewayCanTask(void* parameter) {
  (void)parameter;
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    twaiCanDriverPollHealth();
    mcpCanDriverPollHealth();

    routeHuToMkbd();
    routeMkbdToHu();

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(GDS_TASK_CAN_MS));
  }
}