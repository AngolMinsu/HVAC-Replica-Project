#include "StorageManager.h"

#include <FS.h>
#include <SD_MMC.h>
#include <cstring>
#include <esp_heap_caps.h>
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
  lvglDriver.cache_size = 16U * 1024U;
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

uint64_t storageManagerFileSize(const char* path) {
  if (!storageReady || path == nullptr) return 0;
  File file = SD_MMC.open(normalizePath(path), FILE_READ);
  if (!file || file.isDirectory()) {
    file.close();
    return 0;
  }
  const uint64_t size = file.size();
  file.close();
  return size;
}

bool storageManagerEnsureDirectory(const char* path) {
  if (!storageReady || path == nullptr) return false;
  const char* normalized = normalizePath(path);
  return SD_MMC.exists(normalized) || SD_MMC.mkdir(normalized);
}

bool storageManagerRemoveFile(const char* path) {
  if (!storageReady || path == nullptr) return false;
  const char* normalized = normalizePath(path);
  return !SD_MMC.exists(normalized) || SD_MMC.remove(normalized);
}

bool storageManagerRenameFile(const char* fromPath, const char* toPath) {
  if (!storageReady || fromPath == nullptr || toPath == nullptr) return false;
  return SD_MMC.rename(normalizePath(fromPath), normalizePath(toPath));
}

File storageManagerOpenFile(const char* path, const char* mode) {
  if (!storageReady || path == nullptr || mode == nullptr) return File();
  return SD_MMC.open(normalizePath(path), mode);
}

bool storageManagerLoadLvglImage(const char* lvglPath, lv_img_dsc_t* image,
                                 uint16_t targetWidth, uint16_t targetHeight) {
  static_assert(sizeof(lv_img_header_t) == 4, "Unexpected LVGL image header size");
  if (!storageReady || lvglPath == nullptr || image == nullptr) return false;

  File file = SD_MMC.open(normalizePath(lvglPath), FILE_READ);
  if (!file || file.isDirectory() || file.size() <= sizeof(lv_img_header_t)) {
    file.close();
    return false;
  }

  lv_img_header_t header{};
  if (file.read(reinterpret_cast<uint8_t*>(&header), sizeof(header)) != sizeof(header) ||
      header.always_zero != 0 || header.w == 0 || header.h == 0) {
    file.close();
    return false;
  }

  const size_t dataSize = file.size() - sizeof(header);
  uint8_t* data = static_cast<uint8_t*>(
      heap_caps_malloc(dataSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (data == nullptr) {
    file.close();
    Serial.printf("SD: PSRAM allocation failed for %s\n", lvglPath);
    return false;
  }

  size_t totalRead = 0;
  while (totalRead < dataSize) {
    const size_t remaining = dataSize - totalRead;
    const size_t chunk = remaining > 32768U ? 32768U : remaining;
    const size_t bytesRead = file.read(data + totalRead, chunk);
    if (bytesRead == 0) break;
    totalRead += bytesRead;
  }
  file.close();

  if (totalRead != dataSize) {
    heap_caps_free(data);
    Serial.printf("SD: image read failed for %s\n", lvglPath);
    return false;
  }

  uint8_t* finalData = data;
  size_t finalDataSize = dataSize;
  lv_img_header_t finalHeader = header;

  if (targetWidth > 0 && targetHeight > 0 &&
      (header.w != targetWidth || header.h != targetHeight)) {
    const uint8_t bitsPerPixel = lv_img_cf_get_px_size(header.cf);
    if (bitsPerPixel % 8U != 0) {
      heap_caps_free(data);
      return false;
    }

    const size_t bytesPerPixel = bitsPerPixel / 8U;
    const size_t scaledSize =
        static_cast<size_t>(targetWidth) * targetHeight * bytesPerPixel;
    uint8_t* scaled = static_cast<uint8_t*>(
        heap_caps_malloc(scaledSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (scaled == nullptr) {
      heap_caps_free(data);
      Serial.printf("SD: scaled image allocation failed for %s\n", lvglPath);
      return false;
    }

    for (uint16_t y = 0; y < targetHeight; ++y) {
      const uint16_t sourceY =
          static_cast<uint32_t>(y) * header.h / targetHeight;
      for (uint16_t x = 0; x < targetWidth; ++x) {
        const uint16_t sourceX =
            static_cast<uint32_t>(x) * header.w / targetWidth;
        const size_t sourceOffset =
            (static_cast<size_t>(sourceY) * header.w + sourceX) * bytesPerPixel;
        const size_t targetOffset =
            (static_cast<size_t>(y) * targetWidth + x) * bytesPerPixel;
        memcpy(scaled + targetOffset, data + sourceOffset, bytesPerPixel);
      }
    }

    heap_caps_free(data);
    finalData = scaled;
    finalDataSize = scaledSize;
    finalHeader.w = targetWidth;
    finalHeader.h = targetHeight;
  }

  *image = {};
  image->header = finalHeader;
  image->data_size = static_cast<uint32_t>(finalDataSize);
  image->data = finalData;
  Serial.printf("SD: cached %s (%u bytes)\n",
                lvglPath, static_cast<unsigned>(finalDataSize));
  return true;
}
