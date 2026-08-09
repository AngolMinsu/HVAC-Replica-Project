#pragma once

#include "../can/CanDriver.h"

bool mkbdCanOtaHandleFrame(const CanFrame& frame);
void mkbdCanOtaTick();
bool mkbdCanOtaActive();