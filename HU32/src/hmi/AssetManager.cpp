#include "AssetManager.h"

#include <FS.h>
#include <LittleFS.h>
#include <PNGdec.h>

static TFT_eSPI* pngTft = nullptr;
static fs::File pngFile;
static int16_t pngX = 0;
static int16_t pngY = 0;
static PNG pngDecoder;

static void* pngOpen(const char* filename, int32_t* size) {
  pngFile = LittleFS.open(filename, "r");
  if (!pngFile) {
    return nullptr;
  }

  *size = pngFile.size();
  return &pngFile;
}

static void pngClose(void* handle) {
  fs::File* file = static_cast<fs::File*>(handle);
  if (file) {
    file->close();
  }
}

static int32_t pngRead(PNGFILE* page, uint8_t* buffer, int32_t length) {
  fs::File* file = static_cast<fs::File*>(page->fHandle);
  if (!file) {
    return 0;
  }

  return file->read(buffer, length);
}

static int32_t pngSeek(PNGFILE* page, int32_t position) {
  fs::File* file = static_cast<fs::File*>(page->fHandle);
  if (!file) {
    return 0;
  }

  return file->seek(position);
}

static void pngDraw(PNGDRAW* draw) {
  static uint16_t lineBuffer[480];
  if (!pngTft || draw->iWidth > 480) {
    return;
  }

  pngDecoder.getLineAsRGB565(draw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  pngTft->pushImage(pngX, pngY + draw->y, draw->iWidth, 1, lineBuffer);
}

bool AssetManager::begin() {
  ready = LittleFS.begin(false);
  if (!ready) {
    Serial.println("LittleFS mount failed. Formatting LittleFS...");
    if (LittleFS.format()) {
      Serial.println("LittleFS format complete.");
      ready = LittleFS.begin(false);
    } else {
      Serial.println("LittleFS format failed.");
    }
  }

  Serial.println(ready ? "LittleFS:OK" : "LittleFS:NOK");
  return ready;
}

bool AssetManager::isReady() const {
  return ready;
}

bool AssetManager::drawPng(TFT_eSPI& tft, const char* path, int16_t x, int16_t y) {
  if (!ready || !LittleFS.exists(path)) {
    return false;
  }

  pngTft = &tft;
  pngX = x;
  pngY = y;

  int16_t result = pngDecoder.open(path, pngOpen, pngClose, pngRead, pngSeek, pngDraw);
  if (result != PNG_SUCCESS) {
    return false;
  }

  pngDecoder.decode(nullptr, 0);
  pngDecoder.close();
  return true;
}

void AssetManager::drawFallbackPanel(TFT_eSPI& tft, int16_t x, int16_t y, int16_t w, int16_t h, const char* label, uint16_t borderColor) {
  tft.fillRoundRect(x, y, w, h, 10, TFT_NAVY);
  tft.drawRoundRect(x, y, w, h, 10, borderColor);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(label, x + w / 2, y + h / 2, 2);
  tft.setTextDatum(TL_DATUM);
}
