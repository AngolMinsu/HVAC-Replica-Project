#ifndef HU7_STORAGE_MANAGER_H
#define HU7_STORAGE_MANAGER_H

#include <lvgl.h>

bool storageManagerBegin();
bool storageManagerIsReady();
bool storageManagerFileExists(const char* lvglPath);
bool storageManagerLoadLvglImage(const char* lvglPath, lv_img_dsc_t* image);

#endif