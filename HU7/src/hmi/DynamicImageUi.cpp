#include "DynamicImageUi.h"

#include <lvgl.h>

#include "../storage/StorageManager.h"

extern "C" {
#include "../generated/squareline/ui.h"
}

namespace {

constexpr const char* kSettingsImage = "S:/assets/ui/Home_setting.bin";
constexpr const char* kMapCardImage = "S:/assets/ui/Home_Map.bin";
constexpr const char* kMediaCardImage = "S:/assets/ui/Home_Media.bin";
constexpr const char* kMapImage = "S:/assets/map/NaverMap.bin";
constexpr const char* kNoMediaImage = "S:/assets/media/No_Media.bin";

lv_obj_t* addImage(lv_obj_t* parent, const char* source) {
  if (parent == nullptr || !storageManagerFileExists(source)) return nullptr;
  lv_obj_t* image = lv_img_create(parent);
  lv_img_set_src(image, source);
  lv_obj_clear_flag(image, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  return image;
}

void addCardImage(lv_obj_t* card, const char* source) {
  if (card == nullptr) return;
  lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_OVERFLOW_VISIBLE);
  lv_obj_t* image = addImage(card, source);
  if (image == nullptr) return;
  lv_obj_align(image, LV_ALIGN_BOTTOM_MID, 0, -4);
  lv_obj_move_background(image);
}

void addNoMediaImage(lv_obj_t* label) {
  if (label == nullptr) return;
  lv_obj_t* image = addImage(lv_obj_get_parent(label), kNoMediaImage);
  if (image == nullptr) return;
  lv_obj_align_to(image, label, LV_ALIGN_OUT_LEFT_MID, -12, 0);
}

}  // namespace

void dynamicImageUiBegin() {
  if (!storageManagerIsReady()) return;

  addCardImage(ui_CardSet, kSettingsImage);
  addCardImage(ui_CardMap, kMapCardImage);
  addCardImage(ui_CardMedia, kMediaCardImage);

  if (ui_ContentMap != nullptr) {
    lv_obj_clear_flag(ui_ContentMap, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_t* mapImage = addImage(ui_ContentMap, kMapImage);
    if (mapImage != nullptr) {
      lv_obj_center(mapImage);
      lv_obj_move_background(mapImage);
    }
  }

  addNoMediaImage(ui_CurrentMediaTxt);
  addNoMediaImage(ui_CurrentMediaTxt1);
  addNoMediaImage(ui_CurrentMediaTxt2);
  addNoMediaImage(ui_CurrentMediaTxt3);
  addNoMediaImage(ui_CurrentMediaTxt4);
  addNoMediaImage(ui_CurrentMediaTxt5);
}
