#ifndef TEST_FREERTOS_H
#define TEST_FREERTOS_H
#include <stdint.h>
typedef uint32_t TickType_t;
#define pdMS_TO_TICKS(x) (x)
#define portMAX_DELAY 0xffffffffu
#define pdTRUE 1
#endif
