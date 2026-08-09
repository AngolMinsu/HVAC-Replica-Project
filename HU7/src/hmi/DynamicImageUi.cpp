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

lv_img_dsc_t settingsDescriptor{};
lv_img_dsc_t mapCardDescriptor{};
lv_img_dsc_t mapDescriptor{};
lv_img_dsc_t mediaCardDescriptor{};
lv_img_dsc_t noMediaDescriptor{};

const void* resolveImage(const char* path, lv_img_dsc_t* descriptor, bool preload,
                         uint16_t targetWidth = 0, uint16_t targetHeight = 0) {
  if (!storageManagerFileExists(path)) return nullptr;
  if (preload && descriptor != nullptr &&
      storageManagerLoadLvglImage(path, descriptor, targetWidth, targetHeight)) {
    return descriptor;
  }
  return path;
}

lv_obj_t* addImage(lv_obj_t* parent, const void* source) {
  if (parent == nullptr || source == nullptr) return nullptr;
  lv_obj_t* image = lv_img_create(parent);
  lv_img_set_src(image, source);
  lv_obj_clear_flag(image, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  return image;
}

void removeCardBezel(lv_obj_t* card) {
  lv_obj_remove_style_all(card);
  lv_obj_set_style_pad_all(card, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(card, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_width(card, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(card, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(card, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_clip_corner(card, true, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(card, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_OVERFLOW_VISIBLE);
}

void addCardImage(lv_obj_t* card, const void* source) {
  if (card == nullptr || source == nullptr) return;
  removeCardBezel(card);

  lv_obj_t* image = addImage(card, source);
  if (image == nullptr) return;
  lv_obj_update_layout(card);
  const lv_coord_t cardWidth = lv_obj_get_width(card);
  const lv_coord_t cardHeight = lv_obj_get_height(card);
  if (cardWidth > 0 && cardHeight > 0) {
    lv_obj_set_size(image, cardWidth, cardHeight);
  }
  lv_obj_center(image);
  lv_obj_move_background(image);
}

void styleOutlinedLabel(lv_obj_t* label) {
  if (label == nullptr) return;

  static constexpr lv_coord_t offsets[][2] = {
      {-2, -2}, {0, -2}, {2, -2}, {-2, 0},
      {2, 0}, {-2, 2}, {0, 2}, {2, 2},
  };

  lv_obj_t* parent = lv_obj_get_parent(label);
  const char* text = lv_label_get_text(label);
  const lv_font_t* font = lv_obj_get_style_text_font(label, LV_PART_MAIN);

  for (const auto& offset : offsets) {
    lv_obj_t* outline = lv_label_create(parent);
    lv_label_set_text(outline, text);
    lv_obj_set_style_text_font(outline, font, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(outline, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(outline, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(outline, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align_to(outline, label, LV_ALIGN_CENTER, offset[0], offset[1]);
  }

  lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(label, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_move_foreground(label);
}

void addNoMediaImage(lv_obj_t* label, const void* source) {
  if (label == nullptr || source == nullptr) return;
  lv_obj_t* image = addImage(lv_obj_get_parent(label), source);
  if (image == nullptr) return;
  lv_obj_align_to(image, label, LV_ALIGN_OUT_LEFT_MID, -12, 0);
}

}  // namespace

void dynamicImageUiBegin() {
  if (!storageManagerIsReady()) return;

  const void* settingsSource =
      resolveImage(kSettingsImage, &settingsDescriptor, true, 215, 372);
  const void* mapCardSource =
      resolveImage(kMapCardImage, &mapCardDescriptor, true, 215, 372);
  const void* mediaCardSource =
      resolveImage(kMediaCardImage, &mediaCardDescriptor, true, 215, 372);
  const void* noMediaSource = resolveImage(kNoMediaImage, &noMediaDescriptor, true);

  addCardImage(ui_CardSet, settingsSource);
  addCardImage(ui_CardMap, mapCardSource);
  addCardImage(ui_CardMedia, mediaCardSource);

  styleOutlinedLabel(ui_Settings);
  styleOutlinedLabel(ui_Map);
  styleOutlinedLabel(ui_Media);

  if (ui_ContentMap != nullptr) {
    lv_obj_clear_flag(ui_ContentMap, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    const void* mapSource = resolveImage(kMapImage, &mapDescriptor, true);
    lv_obj_t* mapImage = addImage(ui_ContentMap, mapSource);
    if (mapImage != nullptr) {
      lv_obj_center(mapImage);
      lv_obj_move_background(mapImage);
    }
  }

  addNoMediaImage(ui_CurrentMediaTxt, noMediaSource);
  addNoMediaImage(ui_CurrentMediaTxt1, noMediaSource);
  addNoMediaImage(ui_CurrentMediaTxt2, noMediaSource);
  addNoMediaImage(ui_CurrentMediaTxt3, noMediaSource);
  addNoMediaImage(ui_CurrentMediaTxt4, noMediaSource);
  addNoMediaImage(ui_CurrentMediaTxt5, noMediaSource);
}
