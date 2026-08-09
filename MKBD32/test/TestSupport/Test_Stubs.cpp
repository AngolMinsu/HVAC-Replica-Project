#include "Arduino.h"
#include "Wire.h"
#include "driver/twai.h"
#include "freertos/task.h"
#include "../../can/CanDriver.h"

uint8_t testPinLevel[64] = {};
int testAnalogValue[64] = {};
unsigned long testMillis = 0;
TestSerial Serial;
TestWire Wire;
esp_err_t testTwaiInstallResult=ESP_OK;
esp_err_t testTwaiStartResult=ESP_OK;
esp_err_t testTwaiTxResult=ESP_OK;
esp_err_t testTwaiRxResult=ESP_FAIL;
twai_message_t testTwaiTx={};
twai_message_t testTwaiRx={};
int testTwaiTxCount=0;
int testTaskCreateCount=0;

bool mkbdCanOtaHandleFrame(const CanFrame&) { return false; }
void mkbdCanOtaTick() {}
bool mkbdCanOtaActive() { return false; }
