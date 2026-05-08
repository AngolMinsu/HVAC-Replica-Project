#ifndef HU_WEB_UI_H
#define HU_WEB_UI_H

#include <Arduino.h>
#include "CanProtocol.h"

typedef uint8_t (*HuRequestSender)(uint8_t service, uint8_t signal, uint8_t value);

void webUiBegin(HuRequestSender sender);
void webUiHandle();
void webUiUpdateFromPayload(const CanPayload& payload);

#endif
