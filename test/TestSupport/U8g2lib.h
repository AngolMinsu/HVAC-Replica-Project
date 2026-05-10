#ifndef TEST_U8G2LIB_H
#define TEST_U8G2LIB_H

#include <stdint.h>

static const unsigned char u8g2_font_6x12_tr[] = {0};

class U8G2_SH1106_128X64_NONAME_F_HW_I2C {
 public:
  void clearBuffer() {}
  void setFont(const unsigned char*) {}
  void drawStr(int, int, const char*) {}
  void setCursor(int, int) {}
  void print(const char*) {}
  void print(int) {}
  void drawBox(int, int, int, int) {}
  void drawFrame(int, int, int, int) {}
  void sendBuffer() {}
};

#endif
