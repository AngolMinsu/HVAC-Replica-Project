#include "StorageManager.h"

#include <FS.h>
#include <SD_MMC.h>
#include <cstring>
#include <lvgl.h>

#include "../../GDS.h"
#include "../vendor/waveshare_7b/io_extension.h"

namespace {

bool storageReady = false;
lv_fs_drv_t lvglDriver;

const char* normalizePath(const char* path) {
  if (path == nullptr) return nullptr;
  const char* colon = strchr(path, ':');
  return colon == nullptr ? path : colon + 1;
}

bool fsReady(lv_fs_drv_t*) {
  return storageReady;
}

void* fsOpen(lv_fs_drv_t*, const char* path, lv_fs_mode_t mode) {
  if (!storageReady || path == nullptr || (mode & LV_FS_MODE_WR) != 0) return nullptr;
  File file = SD_MMC.open(normalizePath(path), FILE_READ);
  if (!file || file.isDirectory()) {
    file.close();
    return nullptr;
  }
  return new File(file);
}

lv_fs_res_t fsClose(lv_fs_drv_t*, void* filePointer) {
  File* file = static_cast<File*>(filePointer);
  if (file == nullptr) return LV_FS_RES_INV_PARAM;
  file->close();
  delete file;
  return LV_FS_RES_OK;
}

lv_fs_res_t fsRead(lv_fs_drv_t*, void* filePointer, void* buffer,
                   uint32_t bytesToRead, uint32_t* bytesRead) {
  File* file = static_cast<File*>(filePointer);
  if (file == nullptr || buffer == nullptr || bytesRead == nullptr) return LV_FS_RES_INV_PARAM;
  *bytesRead = static_cast<uint32_t>(file->read(static_cast<uint8_t*>(buffer), bytesToRead));
  return LV_FS_RES_OK;
}

lv_fs_res_t fsSeek(lv_fs_drv_t*, void* filePointer, uint32_t position,
                   lv_fs_whence_t whence) {
  File* file = static_cast<File*>(filePointer);
  if (file == nullptr) return LV_FS_RES_INV_PARAM;
  SeekMode mode = SeekSet;
  if (whence == LV_FS_SEEK_CUR) mode = SeekCur;
  if (whence == LV_FS_SEEK_END) mode = SeekEnd;
  return file->seek(position, mode) ? LV_FS_RES_OK : LV_FS_RES_FS_ERR;
}

lv_fs_res_t fsTell(lv_fs_drv_t*, void* filePointer, uint32_t* position) {
  File* file = static_cast<File*>(filePointer);
  if (file == nullptr || position == nullptr) return LV_FS_RES_INV_PARAM;
  *position = static_cast<uint32_t>(file->position());
  return LV_FS_RES_OK;
}

void registerLvglDrive() {
  lv_fs_drv_init(&lvglDriver);
  lvglDriver.letter = 'S';
  lvglDriver.ready_cb = fsReady;
  lvglDriver.open_cb = fsOpen;
  lvglDriver.close_cb = fsClose;
  lvglDriver.read_cb = fsRead;
  lvglDriver.seek_cb = fsSeek;
  lvglDriver.tell_cb = fsTell;
  lv_fs_drv_register(&lvglDriver);
}

}  // namespace

bool storageManagerBegin() {
  IO_EXTENSION_Output(GDS_IO_SD_SELECT, 1);
  delay(10);

  if (!SD_MMC.setPins(GDS_PIN_SD_CLK, GDS_PIN_SD_CMD, GDS_PIN_SD_D0)) {
    Serial.println("SD: pin setup failed");
    return false;
  }

  storageReady = SD_MMC.begin("/sdcard", true, false);
  if (!storageReady || SD_MMC.cardType() == CARD_NONE) {
    storageReady = false;
    Serial.println("SD: mount failed");
    return false;
  }

  registerLvglDrive();
  Serial.printf("SD: mounted, %llu MB\n",
                static_cast<unsigned long long>(SD_MMC.cardSize() / (1024ULL * 1024ULL)));
  return true;
}

bool storageManagerIsReady() {
  return storageReady;
}

bool storageManagerFileExists(const char* lvglPath) {
  if (!storageReady || lvglPath == nullptr) return false;
  return SD_MMC.exists(normalizePath(lvglPath));
}
