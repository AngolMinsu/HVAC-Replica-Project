#ifndef TEST_SEMPHR_H
#define TEST_SEMPHR_H
typedef void* SemaphoreHandle_t;
inline SemaphoreHandle_t xSemaphoreCreateMutex(){return (void*)1;}
inline int xSemaphoreTake(SemaphoreHandle_t,unsigned long){return 1;}
inline void xSemaphoreGive(SemaphoreHandle_t){}
#endif
