#include "DisplayDriver.h"

#include <Arduino.h>

#include "../vendor/waveshare_7b/lvgl_port.h"

uint8_t displayDriverBegin() {
  Serial.println("Initializing Waveshare ESP32-S3 Touch LCD 7B");

  esp_lcd_touch_handle_t touchHandle = touch_gt911_init();
  if (touchHandle == nullptr) {
    Serial.println("touch_gt911_init failed");
    return 0;
  }

  esp_lcd_panel_handle_t panelHandle = waveshare_esp32_s3_rgb_lcd_init();
  if (panelHandle == nullptr) {
    Serial.println("waveshare_esp32_s3_rgb_lcd_init failed");
    return 0;
  }

  wavesahre_rgb_lcd_bl_on();

  Serial.println("Initializing LVGL port");
  if (lvgl_port_init(panelHandle, touchHandle) != ESP_OK) {
    Serial.println("lvgl_port_init failed");
    return 0;
  }

  return 1;
}

void displayDriverLoop() {
  delay(5);
}

uint8_t displayDriverLock(int timeoutMs) {
  return lvgl_port_lock(timeoutMs) ? 1 : 0;
}

void displayDriverUnlock() {
  lvgl_port_unlock();
}
