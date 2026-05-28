#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <stdint.h>

uint8_t displayDriverBegin();
void displayDriverLoop();
uint8_t displayDriverLock(int timeoutMs);
void displayDriverUnlock();

#endif
