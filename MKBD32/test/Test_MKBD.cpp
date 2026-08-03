#include <stdio.h>
#include <string.h>

#include "../can/CanProtocol.h"
#include "../app/AppLogic.h"
#include "../app/MkbdHardware.h"
#include "../button/ButtonInput.h"
#include "../can/CanDriver.h"
#include "../can/CanHandler.h"
#include "../can/CanMonitor.h"
#include "../can/MkbdCanService.h"
#include "../display/Datc.h"
#include "../display/Info.h"
#include "../encoder/EncoderInput.h"
#include "../state/State.h"
#include "../task/MkbdRtos.h"
#include "../task/task10ms/can/CanRxTask.h"
#include "../task/task10ms/input/InputTask.h"
#include "../task/task10ms/output/OutputTask.h"
#include "../task/task100ms/display/DisplayTask.h"
#include <driver/twai.h>
#include <freertos/task.h>

#define TEST_ASSERT_RETURN 0
#include "TestSupport/Test_Assert.h"

uint8_t Test_CanPayloadCreate() {
  CanPayload payload = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL,
                                      CAN_SIGNAL_FAN_SPEED, 8, CAN_ERROR_NONE, 7);
  ASSERT_EQUALS(0, payload.service, CAN_SERVICE_WRITE_REQUEST);
  ASSERT_EQUALS(1, payload.result, CAN_RESULT_NORMAL);
  ASSERT_EQUALS(2, payload.signal, CAN_SIGNAL_FAN_SPEED);
  ASSERT_EQUALS(3, payload.value, 8);
  ASSERT_EQUALS(4, payload.option, CAN_ERROR_NONE);
  ASSERT_EQUALS(5, payload.reserved, 0);
  ASSERT_EQUALS(6, payload.counter, 7);
  puts("[PASS] CAN payload create");
  return 1;
}

uint8_t Test_CanChecksum() {
  CanPayload payload = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL,
                                      CAN_SIGNAL_FAN_SPEED, 8, CAN_ERROR_NONE, 7);
  ASSERT_EQUALS(0, canValidateChecksum(payload), 1);
  payload.checksum ^= 1;
  ASSERT_EQUALS(1, canValidateChecksum(payload), 0);
  puts("[PASS] CAN checksum validation");
  return 1;
}

uint8_t Test_CanSerialization() {
  CanPayload payload = canMakePayload(CAN_SERVICE_WRITE_REQUEST, CAN_RESULT_NORMAL,
                                      CAN_SIGNAL_FAN_SPEED, 8, CAN_ERROR_NONE, 7);
  uint8_t bytes[CAN_DLC];
  canPayloadToBytes(payload, bytes);
  CanPayload parsed = canPayloadFromBytes(bytes);
  ASSERT_EQUALS(0, memcmp(&payload, &parsed, sizeof(payload)), 0);
  puts("[PASS] CAN payload serialization");
  return 1;
}

uint8_t Test_StateInitialization() {
  SystemState state;
  initSystemState(state);
  ASSERT_EQUALS(0, state.screenMode, SCREEN_DATC);
  ASSERT_EQUALS(1, state.fanSpeed, GDS_FAN_SPEED_MIN);
  ASSERT_EQUALS(2, state.driverTemp, GDS_TEMP_DEFAULT);
  ASSERT_EQUALS(3, state.passengerTemp, GDS_TEMP_DEFAULT);
  ASSERT_EQUALS(4, state.volume, GDS_VOLUME_DEFAULT);
  ASSERT_EQUALS(5, state.mediaIndex, GDS_MEDIA_INDEX_DEFAULT);
  puts("[PASS] system state initialization");
  return 1;
}

uint8_t Test_ScreenToggle() {
  SystemState state;
  initSystemState(state);
  toggleScreenMode(state);
  ASSERT_EQUALS(0, state.screenMode, SCREEN_INFO);
  toggleScreenMode(state);
  ASSERT_EQUALS(1, state.screenMode, SCREEN_DATC);
  puts("[PASS] screen mode toggle");
  return 1;
}

uint8_t Test_StateTextConversion() {
  ASSERT_EQUALS(0, screenModeToText(SCREEN_INFO)[0], 'I');
  ASSERT_EQUALS(1, hvacModeToText(HVAC_AUTO)[0], 'A');
  ASSERT_EQUALS(2, windModeToText(WIND_DEF)[0], 'D');
  puts("[PASS] state text conversion");
  return 1;
}

uint8_t Test_ButtonInput() {
  ButtonHistory h; initButtonHistory(h);
  ButtonLevels levels = {HIGH,HIGH,LOW,HIGH};
  ASSERT_EQUALS(0, detectButtonEvent(h, levels, 100, 60), APP_BUTTON_SCREEN);
  ASSERT_EQUALS(1, detectButtonEvent(h, levels, 110, 60), APP_BUTTON_NONE);
  puts("[PASS] ButtonInput module"); return 1;
}

uint8_t Test_EncoderInput() {
  EncoderHistory h; initEncoderHistory(h);
  EncoderLevels levels = {LOW,HIGH,HIGH,HIGH,HIGH,HIGH};
  ASSERT_EQUALS(0, detectEncoderEvent(h, levels, 10, 5), ENCODER_EVENT_DRIVER_CW);
  puts("[PASS] EncoderInput module"); return 1;
}

uint8_t Test_Datc() {
  SystemState s; initSystemState(s);
  ASSERT_EQUALS(0, datcIncreaseFanSpeed(s), 1);
  ASSERT_EQUALS(1, s.fanSpeed, 1);
  s.driverTemp=GDS_TEMP_MAX; ASSERT_EQUALS(2, datcIncreaseDriverTemp(s), 0);
  ASSERT_EQUALS(3, datcCycleWindMode(s), 1);
  puts("[PASS] Datc module"); return 1;
}

uint8_t Test_Info() {
  SystemState s; initSystemState(s); s.screenMode=SCREEN_INFO;
  ASSERT_EQUALS(0, infoIncreaseVolume(s), 1);
  ASSERT_EQUALS(1, infoToggleMute(s), 1);
  ASSERT_EQUALS(2, infoHandleMedia(s), 1);
  ASSERT_EQUALS(3, infoMediaIndexUp(s), 1);
  puts("[PASS] Info module"); return 1;
}

uint8_t Test_AppLogic() {
  SystemState s; initSystemState(s);
  ASSERT_EQUALS(0, handleButtonAction(s, APP_BUTTON_FAN_UP), 1);
  ASSERT_EQUALS(1, s.fanSpeed, 1);
  ASSERT_EQUALS(2, handleEncoderAction(s, ENCODER_EVENT_DRIVER_CW), 1);
  ASSERT_EQUALS(3, calculateStatusLed(s), HIGH);
  ASSERT_EQUALS(4, calculateFanPwm(s) >= GDS_FAN_PWM_MIN, 1);
  puts("[PASS] AppLogic module"); return 1;
}

uint8_t Test_CanHandler() {
  SystemState s; initSystemState(s); CanPayload response;
  CanPayload request=canMakePayload(CAN_SERVICE_WRITE_REQUEST,CAN_RESULT_NORMAL,CAN_SIGNAL_VOLUME,20,0,1);
  ASSERT_EQUALS(0, canProcessControlRequest(s,request,response), 1);
  ASSERT_EQUALS(1, s.volume, 20);
  ASSERT_EQUALS(2, response.result, CAN_RESULT_SUCCESS);
  request.checksum^=1;
  ASSERT_EQUALS(3, canProcessControlRequest(s,request,response), 0);
  ASSERT_EQUALS(4, response.option, CAN_ERROR_CHECKSUM);
  puts("[PASS] CanHandler module"); return 1;
}

uint8_t Test_CanDriver() {
  testTwaiInstallResult=ESP_OK; testTwaiStartResult=ESP_OK; testTwaiTxResult=ESP_OK;
  ASSERT_EQUALS(0, canDriverBegin(), 1);
  CanFrame tx={CAN_ID_CONTROL_REQUEST,CAN_DLC,{0}};
  ASSERT_EQUALS(1, canDriverSend(tx), 1);
  testTwaiRx={}; testTwaiRx.identifier=CAN_ID_HVAC_STATUS; testTwaiRx.data_length_code=CAN_DLC; testTwaiRxResult=ESP_OK;
  CanFrame rx={}; ASSERT_EQUALS(2, canDriverReceive(rx), 1);
  ASSERT_EQUALS(3, rx.id, CAN_ID_HVAC_STATUS);
  testTwaiRxResult=ESP_FAIL;
  puts("[PASS] CanDriver module"); return 1;
}

uint8_t Test_CanMonitor() {
  CanFrame frame={CAN_ID_HVAC_STATUS,CAN_DLC,{0}};
  canMonitorPrintFrame("TEST",frame);
  puts("[PASS] CanMonitor module"); return 1;
}

uint8_t Test_MkbdCanService() {
  SystemState s; initSystemState(s);
  CanPayload request=canMakePayload(CAN_SERVICE_WRITE_REQUEST,CAN_RESULT_NORMAL,CAN_SIGNAL_FAN_SPEED,3,0,2);
  testTwaiRx={}; testTwaiRx.identifier=CAN_ID_CONTROL_REQUEST; testTwaiRx.data_length_code=CAN_DLC;
  canPayloadToBytes(request,testTwaiRx.data); testTwaiRxResult=ESP_OK; int before=testTwaiTxCount;
  ASSERT_EQUALS(0, mkbdCanServiceProcessReceive(s), 1);
  ASSERT_EQUALS(1, s.fanSpeed, 3);
  ASSERT_EQUALS(2, testTwaiTxCount > before, 1);
  testTwaiRxResult=ESP_FAIL;
  puts("[PASS] MkbdCanService module"); return 1;
}

uint8_t Test_MkbdHardware() {
  for(int i=0;i<64;i++) testPinLevel[i]=HIGH;
  mkbdHardwareBegin(); SystemState s; initSystemState(s); s.fanSpeed=2;
  ASSERT_EQUALS(0, mkbdHardwareUpdateFan(s), calculateFanPwm(s));
  ASSERT_EQUALS(1, mkbdHardwareDrawDisplay(s), SCREEN_DATC);
  puts("[PASS] MkbdHardware module"); return 1;
}

SystemState state;
static uint8_t hookButton,hookEncoder,hookCanChanged; static int hookFan,hookDraw,hookPrint,hookBroadcast;
uint8_t readButtonEvent(){uint8_t v=hookButton;hookButton=APP_BUTTON_NONE;return v;}
uint8_t readEncoderEvent(){uint8_t v=hookEncoder;hookEncoder=ENCODER_EVENT_NONE;return v;}
int updateFanMotor(){hookFan++;return 1;}
uint8_t drawCurrentScreen(){hookDraw++;return 1;}
uint8_t printSystemStatus(const SystemState&){hookPrint++;return 1;}
uint8_t processCanReceive(){uint8_t v=hookCanChanged;hookCanChanged=0;return v;}
uint8_t broadcastButtonMenuEvent(uint8_t,const SystemState&){return 0;}
uint8_t broadcastEncoderSwitchEvent(uint8_t,const SystemState&){return 0;}
uint8_t broadcastChangedHvacStatus(const SystemState&,const SystemState&){hookBroadcast++;return 1;}

uint8_t Test_TaskModules() {
  initSystemState(state); hookButton=APP_BUTTON_FAN_UP; hookCanChanged=1;
  mkbdTask10msInputRun(); mkbdTask10msCanRxRun(); mkbdTask10msOutputRun(); mkbdTask100msDisplayRun();
  ASSERT_EQUALS(0, state.fanSpeed, 1); ASSERT_EQUALS(1, hookBroadcast, 1);
  ASSERT_EQUALS(2, hookPrint >= 2, 1); ASSERT_EQUALS(3, hookFan, 1); ASSERT_EQUALS(4, hookDraw, 1);
  puts("[PASS] Task modules"); return 1;
}

uint8_t Test_MkbdRtos() {
  testTaskCreateCount=0; mkbdRtosStart(); ASSERT_EQUALS(0,testTaskCreateCount,4);
  puts("[PASS] MkbdRtos module"); return 1;
}
