#include "../../../MKBD/display/Info.h"
#include "../../../MKBD/GDS.h"
#define TEST_ASSERT_RETURN 0
#include "../../TestSupport/Test_Assert.h"

uint8_t Test_Info(uint16_t loop) {
  SystemState state;
  state.mediaReady = false;
  state.mapReady = false;
  state.homeReady = false;
  state.mediaMode = false;
  state.mute = false;
  state.mediaIndex = GDS_MEDIA_INDEX_DEFAULT;
  state.volume = GDS_VOLUME_MAX;

  ASSERT_EQUALS(0, infoIncreaseVolume(state), 0);
  ASSERT_EQUALS(1, state.volume, GDS_VOLUME_MAX);
  ASSERT_EQUALS(2, infoToggleMute(state), 1);
  ASSERT_EQUALS(12, state.mute, 1);
  ASSERT_EQUALS(13, infoToggleMute(state), 1);
  ASSERT_EQUALS(14, state.mute, 0);
  ASSERT_EQUALS(3, state.volume, GDS_VOLUME_MAX);

  state.volume = GDS_VOLUME_MIN;
  ASSERT_EQUALS(4, infoDecreaseVolume(state), 0);
  ASSERT_EQUALS(5, state.volume, GDS_VOLUME_MIN);

  ASSERT_EQUALS(6, infoHandleMap(state), 1);
  ASSERT_EQUALS(7, state.mapReady, 1);
  ASSERT_EQUALS(15, infoHandleHome(state), 1);
  ASSERT_EQUALS(16, state.homeReady, 1);
  ASSERT_EQUALS(17, infoMediaIndexUp(state), 0);
  ASSERT_EQUALS(18, infoHandleMedia(state), 1);
  ASSERT_EQUALS(19, state.mediaMode, 1);
  ASSERT_EQUALS(20, infoMediaIndexUp(state), 1);
  ASSERT_EQUALS(21, state.mediaIndex, GDS_MEDIA_INDEX_DEFAULT + 1);
  ASSERT_EQUALS(22, infoMediaIndexDown(state), 1);
  ASSERT_EQUALS(23, state.mediaIndex, GDS_MEDIA_INDEX_DEFAULT);
  ASSERT_EQUALS(24, infoSelect(state), 1);
  ASSERT_EQUALS(25, state.mediaReady, 1);

  state.volume = (GDS_VOLUME_MIN + GDS_VOLUME_MAX) / 2;
  ASSERT_EQUALS(8, infoIncreaseVolume(state), 1);
  ASSERT_EQUALS(9, state.volume, ((GDS_VOLUME_MIN + GDS_VOLUME_MAX) / 2) + 1);
  ASSERT_EQUALS(10, infoDecreaseVolume(state), 1);
  ASSERT_EQUALS(11, state.volume, (GDS_VOLUME_MIN + GDS_VOLUME_MAX) / 2);
  return 1;
}
