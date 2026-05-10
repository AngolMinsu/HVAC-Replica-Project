#include "../../../MKBD/display/Info.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_Info(uint16_t loop) {
  SystemState state;
  state.mediaReady = false;
  state.mapReady = false;
  state.volume = GDS_VOLUME_MAX;

  ASSERT_EQUALS(0, infoHandleButton2(state), 1);
  ASSERT_EQUALS(1, state.mediaReady, 1);
  ASSERT_EQUALS(2, infoHandleButton3(state), 0);
  ASSERT_EQUALS(3, state.volume, GDS_VOLUME_MAX);

  state.volume = GDS_VOLUME_MIN;
  ASSERT_EQUALS(4, infoHandleButton4(state), 0);
  ASSERT_EQUALS(5, state.volume, GDS_VOLUME_MIN);

  ASSERT_EQUALS(6, infoHandleButton5(state), 1);
  ASSERT_EQUALS(7, state.mapReady, 1);

  state.volume = (GDS_VOLUME_MIN + GDS_VOLUME_MAX) / 2;
  ASSERT_EQUALS(8, infoHandleButton3(state), 1);
  ASSERT_EQUALS(9, state.volume, ((GDS_VOLUME_MIN + GDS_VOLUME_MAX) / 2) + 1);
  ASSERT_EQUALS(10, infoHandleButton4(state), 1);
  ASSERT_EQUALS(11, state.volume, (GDS_VOLUME_MIN + GDS_VOLUME_MAX) / 2);
  return 1;
}
