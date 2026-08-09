#ifndef HU7_STORAGE_MANAGER_H
#define HU7_STORAGE_MANAGER_H

bool storageManagerBegin();
bool storageManagerIsReady();
bool storageManagerFileExists(const char* lvglPath);

#endif
