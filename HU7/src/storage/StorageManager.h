#ifndef HU7_STORAGE_MANAGER_H
#define HU7_STORAGE_MANAGER_H

#include <FS.h>
#include <lvgl.h>

bool storageManagerBegin();
bool storageManagerIsReady();
bool storageManagerFileExists(const char* lvglPath);
uint64_t storageManagerFileSize(const char* path);
bool storageManagerEnsureDirectory(const char* path);
bool storageManagerRemoveFile(const char* path);
bool storageManagerRenameFile(const char* fromPath, const char* toPath);
File storageManagerOpenFile(const char* path, const char* mode);
bool storageManagerLoadLvglImage(const char* lvglPath, lv_img_dsc_t* image,
                                 uint16_t targetWidth = 0, uint16_t targetHeight = 0);

#endif
