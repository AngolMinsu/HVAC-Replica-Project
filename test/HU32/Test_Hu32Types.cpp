#include "../../HU32/src/hmi/HuTypes.h"
#define TEST_ASSERT_RETURN 0
#include "../TestSupport/Test_Assert.h"

uint8_t Test_Hu32Types(uint16_t loop) {
  if (loop == 0) loop = 1;

  for (uint16_t i = 0; i < loop; i++) {
    SystemState state;
    initSystemState(state);

    ASSERT_EQUALS(0, state.screen, HU_SCREEN_HOME);
    ASSERT_EQUALS(1, state.focusedPanel, HU_PANEL_MEDIA);
    ASSERT_EQUALS(2, state.driverTemp, 24);
    ASSERT_EQUALS(3, state.passengerTemp, 24);
    ASSERT_EQUALS(4, state.volume, 10);
    ASSERT_EQUALS(5, state.dirtyFlags, DIRTY_FULL);

    CanPayload payload = canMakePayload(CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, CAN_SIGNAL_FAN_SPEED, 7, 0, 1);
    applyCanPayloadToState(state, payload);
    ASSERT_EQUALS(6, state.fanSpeed, 7);
    ASSERT_EQUALS(7, state.lastSignal, CAN_SIGNAL_FAN_SPEED);
    ASSERT_EQUALS(8, state.lastValue, 7);

    payload = canMakePayload(CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, CAN_SIGNAL_TEMPERATURE, 18, 0, 2);
    applyCanPayloadToState(state, payload);
    ASSERT_EQUALS(9, state.driverTemp, 18);

    payload = canMakePayload(CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, CAN_SIGNAL_PASSENGER_TEMPERATURE, 27, 0, 3);
    applyCanPayloadToState(state, payload);
    ASSERT_EQUALS(10, state.passengerTemp, 27);

    payload = canMakePayload(CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, CAN_SIGNAL_VOLUME, 15, 0, 4);
    applyCanPayloadToState(state, payload);
    ASSERT_EQUALS(11, state.volume, 15);

    payload = canMakePayload(CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, CAN_SIGNAL_MUTE, 1, 0, 5);
    applyCanPayloadToState(state, payload);
    ASSERT_EQUALS(12, state.mute, 1);

    payload = canMakePayload(CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, CAN_SIGNAL_MEDIA_INDEX, 4, 0, 6);
    applyCanPayloadToState(state, payload);
    ASSERT_EQUALS(13, state.mediaIndex, 4);
    ASSERT_EQUALS(14, state.media.title[0], 'T');

    payload = canMakePayload(CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_FAIL, CAN_SIGNAL_FAN_SPEED, 1, 0, 7);
    applyCanPayloadToState(state, payload);
    ASSERT_EQUALS(15, state.fanSpeed, 7);

    payload = canMakePayload(CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, CAN_SIGNAL_FAN_SPEED, 3, 0, 8);
    payload.checksum ^= 0x01;
    applyCanPayloadToState(state, payload);
    ASSERT_EQUALS(16, state.fanSpeed, 7);

    ASSERT_EQUALS(17, huScreenToText(HU_SCREEN_HOME)[0], 'H');
    ASSERT_EQUALS(18, huWindToText(3)[0], 'M');
    ASSERT_EQUALS(19, dirtyForScreen(HU_SCREEN_MEDIA), DIRTY_MEDIA);
  }

  return 1;
}
