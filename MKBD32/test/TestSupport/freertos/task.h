#ifndef TEST_TASK_H
#define TEST_TASK_H
#include "FreeRTOS.h"
extern int testTaskCreateCount;
inline TickType_t xTaskGetTickCount(){return 0;}
inline void vTaskDelayUntil(TickType_t*,TickType_t){}
inline void vTaskDelay(TickType_t){}
inline int xTaskCreatePinnedToCore(void(*)(void*),const char*,uint32_t,void*,uint32_t,void*,int){testTaskCreateCount++;return 1;}
#endif
