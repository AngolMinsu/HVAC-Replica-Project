#include "../../../MKBD/can/CanHandler.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_CanHandler() {
  SystemState state;
  initSystemState(state);

  CanPayload request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 3, 0, 1);
  CanPayload response;
  ASSERT_EQUALS(0, canProcessControlRequest(state, request, response), 1);
  ASSERT_EQUALS(1, state.fanSpeed, 3);
  ASSERT_EQUALS(2, state.hvacMode, HVAC_AUTO);
  ASSERT_EQUALS(3, response.result, CAN_RESULT_SUCCESS);

  request = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_TEMPERATURE, GDS_TEMP_MAX + 1, 0, 2);
  ASSERT_EQUALS(4, canProcessControlRequest(state, request, response), 0);
  ASSERT_EQUALS(5, response.result, CAN_RESULT_FAIL);
  ASSERT_EQUALS(6, response.option, CAN_ERROR_VALUE_OUT_OF_RANGE);

  request = canMakePayload(CAN_SERVICE_READ_REQUEST, CAN_RESULT_NORMAL, CAN_SIGNAL_FAN_SPEED, 0, 0, 3);
  ASSERT_EQUALS(7, canProcessControlRequest(state, request, response), 1);
  ASSERT_EQUALS(8, response.value, 3);

  uint8_t value = 0;
  ASSERT_EQUALS(9, canSignalValueFromState(state, CAN_SIGNAL_FAN_SPEED, value), 1);
  ASSERT_EQUALS(10, value, 3);
  return 1;
}
