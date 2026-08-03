#ifndef TEST_ARDUINO_H
#define TEST_ARDUINO_H
#include <stdint.h>
#define HIGH 1
#define LOW 0
#define INPUT_PULLUP 2
#define OUTPUT 1
#define HEX 16
extern uint8_t testPinLevel[64];
extern int testAnalogValue[64];
extern unsigned long testMillis;
inline void pinMode(uint8_t, uint8_t) {}
inline int digitalRead(uint8_t pin) { return testPinLevel[pin]; }
inline void analogWrite(uint8_t pin, int value) { testAnalogValue[pin] = value; }
inline unsigned long millis() { return testMillis; }
class TestSerial { public: void begin(unsigned long) {} template<typename T> void print(T) {} template<typename T> void print(T,int) {} template<typename T> void println(T) {} void println() {} };
extern TestSerial Serial;
#endif
