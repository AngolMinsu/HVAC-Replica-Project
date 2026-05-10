#include "../../../MKBD/can/CanHandler.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

static uint8_t expectCanNok(
    SystemState& state,
    const CanPayload& request,
    uint8_t expErrorCode,
    int expFanSpeed,
    int expSetTemp,
    int expVolume) {
  CanPayload response;
  ASSERT_EQUALS(0, canProcessControlRequest(state, request, response), 0);
  ASSERT_EQUALS(1, response.result, CAN_RESULT_FAIL);
  ASSERT_EQUALS(2, response.option, expErrorCode);
  ASSERT_EQUALS(3, state.fanSpeed, expFanSpeed);
  ASSERT_EQUALS(4, state.setTemp, expSetTemp);
  ASSERT_EQUALS(5, state.volume, expVolume);
  return 1;
}

uint8_t Test_CanHandler(uint16_t loop) {
  if (loop == 0) loop = 1;

  SystemState state;
  initSystemState(state);

  CanPayload response;
  CanPayload request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, GDS_FAN_SPEED_MIN, 0, 1);
  ASSERT_EQUALS(0, canProcessControlRequest(state, request, response), 1);
  ASSERT_EQUALS(1, state.fanSpeed, GDS_FAN_SPEED_MIN);
  ASSERT_EQUALS(2, state.hvacMode, HVAC_OFF);

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, (GDS_FAN_SPEED_MIN + GDS_FAN_SPEED_MAX) / 2, 0, 2);
  ASSERT_EQUALS(3, canProcessControlRequest(state, request, response), 1);
  ASSERT_EQUALS(4, state.fanSpeed, (GDS_FAN_SPEED_MIN + GDS_FAN_SPEED_MAX) / 2);
  ASSERT_EQUALS(5, state.hvacMode, HVAC_AUTO);

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, GDS_FAN_SPEED_MAX, 0, 3);
  ASSERT_EQUALS(6, canProcessControlRequest(state, request, response), 1);
  ASSERT_EQUALS(7, state.fanSpeed, GDS_FAN_SPEED_MAX);
  ASSERT_EQUALS(8, response.result, CAN_RESULT_SUCCESS);

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_TEMPERATURE, GDS_TEMP_MIN, 0, 4);
  ASSERT_EQUALS(9, canProcessControlRequest(state, request, response), 1);
  ASSERT_EQUALS(10, state.setTemp, GDS_TEMP_MIN);

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_TEMPERATURE, GDS_TEMP_MAX, 0, 5);
  ASSERT_EQUALS(11, canProcessControlRequest(state, request, response), 1);
  ASSERT_EQUALS(12, state.setTemp, GDS_TEMP_MAX);

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_TEMPERATURE, GDS_TEMP_MAX + 1, 0, 6);
  ASSERT_EQUALS(13, canProcessControlRequest(state, request, response), 0);
  ASSERT_EQUALS(14, response.result, CAN_RESULT_FAIL);
  ASSERT_EQUALS(15, response.option, CAN_ERROR_VALUE_OUT_OF_RANGE);

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_TEMPERATURE, 0xFF, 0, 7);
  ASSERT(expectCanNok(state, request, CAN_ERROR_VALUE_OUT_OF_RANGE, GDS_FAN_SPEED_MAX, GDS_TEMP_MAX, GDS_VOLUME_DEFAULT));

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_VOLUME, GDS_VOLUME_MAX, 0, 7);
  ASSERT_EQUALS(16, canProcessControlRequest(state, request, response), 1);
  ASSERT_EQUALS(17, state.volume, GDS_VOLUME_MAX);

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_VOLUME, GDS_VOLUME_MAX + 1, 0, 8);
  ASSERT_EQUALS(18, canProcessControlRequest(state, request, response), 0);
  ASSERT_EQUALS(19, response.option, CAN_ERROR_VALUE_OUT_OF_RANGE);

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 0xFF, 0, 9);
  ASSERT(expectCanNok(state, request, CAN_ERROR_VALUE_OUT_OF_RANGE, GDS_FAN_SPEED_MAX, GDS_TEMP_MAX, GDS_VOLUME_MAX));

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_VOLUME, 0xFF, 0, 10);
  ASSERT(expectCanNok(state, request, CAN_ERROR_VALUE_OUT_OF_RANGE, GDS_FAN_SPEED_MAX, GDS_TEMP_MAX, GDS_VOLUME_MAX));

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, 0xFF, 1, 0, 11);
  ASSERT(expectCanNok(state, request, CAN_ERROR_UNSUPPORTED_SIGNAL, GDS_FAN_SPEED_MAX, GDS_TEMP_MAX, GDS_VOLUME_MAX));

  request = canMakePayload(CAN_SERVICE_READ_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 0, 0, 9);
  ASSERT_EQUALS(20, canProcessControlRequest(state, request, response), 1);
  ASSERT_EQUALS(21, response.value, GDS_FAN_SPEED_MAX);

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 3, 0, 10);
  request.checksum ^= 0x01;
  ASSERT_EQUALS(22, canProcessControlRequest(state, request, response), 0);
  ASSERT_EQUALS(23, response.option, CAN_ERROR_CHECKSUM);

  uint8_t value = 0;
  ASSERT_EQUALS(24, canSignalValueFromState(state, CAN_SIGNAL_FAN_SPEED, value), 1);
  ASSERT_EQUALS(25, value, GDS_FAN_SPEED_MAX);
  return 1;
}
