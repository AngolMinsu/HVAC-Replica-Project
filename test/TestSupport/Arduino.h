#ifndef TEST_ARDUINO_H
#define TEST_ARDUINO_H

#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>

#define HEX 16

#ifndef HIGH
#define HIGH 1
#endif

#ifndef LOW
#define LOW 0
#endif

#ifndef D8
#define D8 15
#endif

static inline int isDigit(char c) {
  return isdigit((unsigned char)c);
}

static inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

class String {
 public:
  String() {}
  String(const char* text) : value_(text ? text : "") {}
  String(const std::string& text) : value_(text) {}

  void trim() {
    size_t first = value_.find_first_not_of(" \t\r\n");
    size_t last = value_.find_last_not_of(" \t\r\n");
    if (first == std::string::npos) {
      value_.clear();
      return;
    }
    value_ = value_.substr(first, last - first + 1);
  }

  void toLowerCase() {
    for (size_t i = 0; i < value_.size(); ++i) {
      value_[i] = (char)tolower((unsigned char)value_[i]);
    }
  }

  int indexOf(char c) const {
    size_t found = value_.find(c);
    return found == std::string::npos ? -1 : (int)found;
  }

  String substring(int start) const {
    if (start < 0 || (size_t)start >= value_.size()) return String("");
    return String(value_.substr((size_t)start));
  }

  String substring(int start, int end) const {
    if (start < 0) start = 0;
    if (end < start) end = start;
    return String(value_.substr((size_t)start, (size_t)(end - start)));
  }

  uint16_t length() const {
    return (uint16_t)value_.size();
  }

  char charAt(uint16_t index) const {
    return index < value_.size() ? value_[index] : '\0';
  }

  int toInt() const {
    return atoi(value_.c_str());
  }

  String& operator=(const char* text) {
    value_ = text ? text : "";
    return *this;
  }

  bool operator==(const char* text) const {
    return value_ == (text ? text : "");
  }

  bool operator!=(const char* text) const {
    return !(*this == text);
  }

 private:
  std::string value_;
};

class TestSerial {
 public:
  void println() {}
  void println(const char*) {}
  void println(int) {}
  void println(unsigned int) {}
  void println(unsigned long) {}
  void print(const char*) {}
  void print(char) {}
  void print(int) {}
  void print(unsigned int) {}
  void print(unsigned long) {}
  void print(uint8_t value, int) { (void)value; }
  void print(uint16_t value, int) { (void)value; }
  void print(unsigned long value, int) { (void)value; }
};

static TestSerial Serial;

#endif
