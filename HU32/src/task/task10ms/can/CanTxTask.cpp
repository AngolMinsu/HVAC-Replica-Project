#include "CanTxTask.h"

#include "../../../can/CanDriver.h"
#include "../../../can/CanMonitor.h"
#include "../../../can/CanProtocol.h"
#include "../../../hmi/HuRtosContext.h"

void huCanTxTask(void* parameter) {
  HuRtosContext* context = static_cast<HuRtosContext*>(parameter);

  for (;;) {
    HuCanCommand command;
    if (xQueueReceive(context->canTxQueue, &command, portMAX_DELAY) == pdTRUE) {
      CanPayload payload = canMakePayload(command.service,
                                          CAN_RESULT_NORMAL,
                                          command.signal,
                                          command.value,
                                          0,
                                          (*context->huTxCounter)++);
      CanFrame frame;
      frame.id = CAN_ID_CONTROL_REQUEST;
      frame.dlc = CAN_DLC;
      canPayloadToBytes(payload, frame.data);

      canMonitorPrintFrame("TX", frame);
      canMonitorPrintPayloadSummary(payload);
      canDriverSend(frame);
    }
  }
}
