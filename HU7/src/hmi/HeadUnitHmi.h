#ifndef HEAD_UNIT_HMI_H
#define HEAD_UNIT_HMI_H

#include "../can/CanDriver.h"
#include "../can/CanProtocol.h"

void headUnitHmiBegin();
void headUnitHmiUpdateClock();
void headUnitHmiApplyCanPayload(const CanPayload& payload);
void headUnitHmiOpenHome();
void headUnitHmiOpenMap();
void headUnitHmiOpenSetting();
void headUnitHmiOpenSettingInfo();
void headUnitHmiTick();

#endif
