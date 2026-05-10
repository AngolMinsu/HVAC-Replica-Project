#include "../../HU/CommandParser.h"
#include "../../MKBD/can/CanHandler.h"
#include "../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../TestSupport/Test_Assert.h"

static CanPayload makeRequestFromHuCommand(const HuCommand& command, uint8_t counter) {
  return canMakePayload(command.service, CAN_RESULT_NORMAL, command.signal, command.value, 0, counter);
}

static uint8_t applyHuInput(
    SystemState& state,
    uint16_t loop,
    const char* inputLine,
    uint8_t counter,
    uint8_t expProcessOk,
    uint8_t expResponseService,
    uint8_t expResponseResult,
    uint8_t expResponseValue,
    ScreenMode expScreenMode,
    HvacMode expHvacMode,
    int expFanSpeed,
    int expSetTemp,
    WindMode expWindMode,
    int expVolume,
    uint8_t expMediaReady,
    uint8_t expMapReady) {
  if (loop == 0) loop = 1;

  for (uint16_t i = 0; i < loop; i++) {
    HuCommand command = parseHuCommand(String(inputLine));
    ASSERT_EQUALS(0, command.valid, 1);

    CanPayload request = makeRequestFromHuCommand(command, (uint8_t)(counter + i));
    ASSERT_EQUALS(1, canValidateChecksum(request), 1);

    uint8_t bytes[CAN_DLC];
    canPayloadToBytes(request, bytes);
    CanPayload received = canPayloadFromBytes(bytes);

    CanPayload response;
    ASSERT_EQUALS(2, canProcessControlRequest(state, received, response), expProcessOk);
    ASSERT_EQUALS(3, response.service, expResponseService);
    ASSERT_EQUALS(4, response.result, expResponseResult);
    ASSERT_EQUALS(5, response.value, expResponseValue);
    ASSERT_EQUALS(6, canValidateChecksum(response), 1);

    ASSERT_EQUALS(7, state.screenMode, expScreenMode);
    ASSERT_EQUALS(8, state.hvacMode, expHvacMode);
    ASSERT_EQUALS(9, state.fanSpeed, expFanSpeed);
    ASSERT_EQUALS(10, state.setTemp, expSetTemp);
    ASSERT_EQUALS(11, state.windMode, expWindMode);
    ASSERT_EQUALS(12, state.volume, expVolume);
    ASSERT_EQUALS(13, state.mediaReady, expMediaReady);
    ASSERT_EQUALS(14, state.mapReady, expMapReady);
  }

  return 1;
}

static uint8_t rejectInvalidHuInput(uint16_t loop, const char* inputLine) {
  if (loop == 0) loop = 1;

  for (uint16_t i = 0; i < loop; i++) {
    HuCommand command = parseHuCommand(String(inputLine));
    ASSERT_EQUALS(0, command.valid, 0);
  }

  return 1;
}

static uint8_t rejectCorruptedCanRequest(uint16_t loop) {
  if (loop == 0) loop = 1;

  for (uint16_t i = 0; i < loop; i++) {
    SystemState state;
    initSystemState(state);

    HuCommand command = parseHuCommand(String("fan 4"));
    ASSERT_EQUALS(0, command.valid, 1);

    CanPayload request = makeRequestFromHuCommand(command, (uint8_t)(30 + i));
    request.checksum ^= 0x01;

    CanPayload response;
    ASSERT_EQUALS(1, canProcessControlRequest(state, request, response), 0);
    ASSERT_EQUALS(2, response.result, CAN_RESULT_FAIL);
    ASSERT_EQUALS(3, response.option, CAN_ERROR_CHECKSUM);
    ASSERT_EQUALS(4, state.fanSpeed, GDS_FAN_SPEED_MIN);
    ASSERT_EQUALS(5, state.hvacMode, HVAC_OFF);
  }

  return 1;
}

uint8_t Test_EndToEndControlInput(uint16_t loop) {
  if (loop == 0) loop = 1;

  SystemState state;
  initSystemState(state);

  ASSERT(applyHuInput(state, loop, "power on", 1, 1, CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, 1,
      SCREEN_DATC, HVAC_AUTO, GDS_FAN_SPEED_MIN + 1, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(applyHuInput(state, 1, "fan 4", 2, 1, CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, 4,
      SCREEN_DATC, HVAC_AUTO, 4, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(applyHuInput(state, 1, "temp 18", 3, 1, CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, GDS_TEMP_MIN,
      SCREEN_DATC, HVAC_AUTO, 4, GDS_TEMP_MIN, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(applyHuInput(state, 1, "temp 24", 4, 1, CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, GDS_TEMP_DEFAULT,
      SCREEN_DATC, HVAC_AUTO, 4, GDS_TEMP_DEFAULT, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(applyHuInput(state, 1, "temp 30", 5, 1, CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, GDS_TEMP_MAX,
      SCREEN_DATC, HVAC_AUTO, 4, GDS_TEMP_MAX, WIND_FACE, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(applyHuInput(state, 1, "wind mix", 6, 1, CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, WIND_MIX,
      SCREEN_DATC, HVAC_AUTO, 4, GDS_TEMP_MAX, WIND_MIX, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(applyHuInput(state, 1, "screen info", 7, 1, CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, SCREEN_INFO,
      SCREEN_INFO, HVAC_AUTO, 4, GDS_TEMP_MAX, WIND_MIX, GDS_VOLUME_DEFAULT, 0, 0));
  ASSERT(applyHuInput(state, 1, "volume 30", 8, 1, CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, GDS_VOLUME_MAX,
      SCREEN_INFO, HVAC_AUTO, 4, GDS_TEMP_MAX, WIND_MIX, GDS_VOLUME_MAX, 0, 0));
  ASSERT(applyHuInput(state, 1, "media ready", 9, 1, CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, 1,
      SCREEN_INFO, HVAC_AUTO, 4, GDS_TEMP_MAX, WIND_MIX, GDS_VOLUME_MAX, 1, 0));
  ASSERT(applyHuInput(state, 1, "map ready", 10, 1, CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, 1,
      SCREEN_INFO, HVAC_AUTO, 4, GDS_TEMP_MAX, WIND_MIX, GDS_VOLUME_MAX, 1, 1));
  ASSERT(applyHuInput(state, 1, "read fan", 11, 1, CAN_SERVICE_READ_RESPONSE, CAN_RESULT_SUCCESS, 4,
      SCREEN_INFO, HVAC_AUTO, 4, GDS_TEMP_MAX, WIND_MIX, GDS_VOLUME_MAX, 1, 1));

  ASSERT(rejectInvalidHuInput(1, "fan 9"));
  ASSERT(rejectInvalidHuInput(1, "fan 255"));
  ASSERT(rejectInvalidHuInput(1, "temp 17"));
  ASSERT(rejectInvalidHuInput(1, "temp 255"));
  ASSERT(rejectInvalidHuInput(1, "volume 31"));
  ASSERT(rejectInvalidHuInput(1, "volume 255"));
  ASSERT(rejectCorruptedCanRequest(1));

  return 1;
}
