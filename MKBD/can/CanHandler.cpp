#include "CanHandler.h"
#include "../GDS.h"

static uint8_t isValidFanSpeed(uint8_t value) {
  return value <= GDS_FAN_SPEED_MAX;
}

static uint8_t isValidTemperature(uint8_t value) {
  return value >= GDS_TEMP_MIN && value <= GDS_TEMP_MAX;
}

static uint8_t isValidWindMode(uint8_t value) {
  return value <= WIND_OFF;
}

static uint8_t isValidScreenMode(uint8_t value) {
  return value <= SCREEN_INFO;
}

static uint8_t isValidVolume(uint8_t value) {
  return value <= GDS_VOLUME_MAX;
}

static uint8_t isValidRadioTune(uint8_t value) {
  return value <= GDS_RADIO_TUNE_MAX;
}

static uint8_t isPowerOn(const SystemState& state) {
  return state.hvacMode == HVAC_OFF ? 0 : 1;
}

static uint8_t isKnownSignal(uint8_t signal) {
  return signal >= CAN_SIGNAL_POWER && signal <= CAN_SIGNAL_RADIO_TUNE;
}

static CanPayload canMakeFailResponse(const CanPayload& request, uint8_t service, uint8_t errorCode) {
  return canMakePayload(service, CAN_RESULT_FAIL, request.signal, request.value, errorCode, request.counter);
}

uint8_t canSignalValueFromState(const SystemState& state, uint8_t signal, uint8_t& value) {
  switch (signal) {
    case CAN_SIGNAL_POWER:
      value = isPowerOn(state);
      return 1;

    case CAN_SIGNAL_FAN_SPEED:
      value = (uint8_t)state.fanSpeed;
      return 1;

    case CAN_SIGNAL_TEMPERATURE:
      // v0.2: CAN Temperature represents the driver-side DATC temperature.
      value = (uint8_t)state.driverTemp;
      return 1;

    case CAN_SIGNAL_PASSENGER_TEMPERATURE:
      value = (uint8_t)state.passengerTemp;
      return 1;

    case CAN_SIGNAL_MODE:
      value = (uint8_t)state.windMode;
      return 1;

    case CAN_SIGNAL_AC:
      value = state.hvacMode == HVAC_AC ? 1 : 0;
      return 1;

    case CAN_SIGNAL_AUTO:
      value = state.hvacMode == HVAC_AUTO ? 1 : 0;
      return 1;

    case CAN_SIGNAL_INTAKE:
      value = 0;
      return 1;

    case CAN_SIGNAL_SCREEN_MODE:
      value = (uint8_t)state.screenMode;
      return 1;

    case CAN_SIGNAL_MEDIA:
      value = state.mediaReady ? 1 : 0;
      return 1;

    case CAN_SIGNAL_VOLUME:
      value = (uint8_t)state.volume;
      return 1;

    case CAN_SIGNAL_MAP:
      value = state.mapReady ? 1 : 0;
      return 1;

    case CAN_SIGNAL_MUTE:
      value = state.mute ? 1 : 0;
      return 1;

    case CAN_SIGNAL_NAV:
      value = state.navReady ? 1 : 0;
      return 1;

    case CAN_SIGNAL_RADIO_MODE:
      value = state.radioMode ? 1 : 0;
      return 1;

    case CAN_SIGNAL_RADIO_TUNE:
      value = (uint8_t)state.radioTune;
      return 1;

    default:
      value = 0;
      return 0;
  }
}

uint8_t canApplyWriteRequest(SystemState& state, const CanPayload& request) {
  if (canValidateWriteRequest(request) != CAN_ERROR_NONE) {
    return 0;
  }

  switch (request.signal) {
    case CAN_SIGNAL_POWER:
      if (request.value > 1) return 0;
      if (request.value == 0) {
        state.hvacMode = HVAC_OFF;
        state.fanSpeed = GDS_FAN_SPEED_MIN;
      } else if (state.hvacMode == HVAC_OFF) {
        state.hvacMode = HVAC_AUTO;
        state.fanSpeed = GDS_FAN_SPEED_MIN + 1;
      }
      return 1;

    case CAN_SIGNAL_FAN_SPEED:
      if (!isValidFanSpeed(request.value)) return 0;
      state.fanSpeed = request.value;
      if (state.fanSpeed == GDS_FAN_SPEED_MIN) {
        state.hvacMode = HVAC_OFF;
      } else if (state.hvacMode == HVAC_OFF) {
        state.hvacMode = HVAC_AUTO;
      }
      return 1;

    case CAN_SIGNAL_TEMPERATURE:
      if (!isValidTemperature(request.value)) return 0;
      state.driverTemp = request.value;
      return 1;

    case CAN_SIGNAL_PASSENGER_TEMPERATURE:
      if (!isValidTemperature(request.value)) return 0;
      state.passengerTemp = request.value;
      return 1;

    case CAN_SIGNAL_MODE:
      if (!isValidWindMode(request.value)) return 0;
      state.windMode = (WindMode)request.value;
      return 1;

    case CAN_SIGNAL_AC:
      if (request.value > 1) return 0;
      if (request.value == 1) {
        state.hvacMode = HVAC_AC;
        if (state.fanSpeed == GDS_FAN_SPEED_MIN) state.fanSpeed = GDS_FAN_SPEED_MIN + 1;
      } else if (state.hvacMode == HVAC_AC) {
        state.hvacMode = HVAC_AUTO;
      }
      return 1;

    case CAN_SIGNAL_AUTO:
      if (request.value > 1) return 0;
      if (request.value == 1) {
        state.hvacMode = HVAC_AUTO;
        if (state.fanSpeed == GDS_FAN_SPEED_MIN) state.fanSpeed = GDS_FAN_SPEED_MIN + 1;
      } else if (state.hvacMode == HVAC_AUTO) {
        state.hvacMode = HVAC_OFF;
        state.fanSpeed = GDS_FAN_SPEED_MIN;
      }
      return 1;

    case CAN_SIGNAL_INTAKE:
      return request.value <= 1;

    case CAN_SIGNAL_SCREEN_MODE:
      if (!isValidScreenMode(request.value)) return 0;
      state.screenMode = (ScreenMode)request.value;
      return 1;

    case CAN_SIGNAL_MEDIA:
      if (request.value > 1) return 0;
      state.mediaReady = request.value == 1;
      return 1;

    case CAN_SIGNAL_VOLUME:
      if (!isValidVolume(request.value)) return 0;
      state.volume = request.value;
      return 1;

    case CAN_SIGNAL_MAP:
      if (request.value > 1) return 0;
      state.mapReady = request.value == 1;
      return 1;

    case CAN_SIGNAL_MUTE:
      if (request.value > 1) return 0;
      state.mute = request.value == 1;
      return 1;

    case CAN_SIGNAL_NAV:
      if (request.value > 1) return 0;
      state.navReady = request.value == 1;
      return 1;

    case CAN_SIGNAL_RADIO_MODE:
      if (request.value > 1) return 0;
      state.radioMode = request.value == 1;
      return 1;

    case CAN_SIGNAL_RADIO_TUNE:
      if (!isValidRadioTune(request.value)) return 0;
      state.radioTune = request.value;
      return 1;

    default:
      return 0;
  }
}

uint8_t canValidateWriteRequest(const CanPayload& request) {
  if (!isKnownSignal(request.signal)) {
    return CAN_ERROR_UNSUPPORTED_SIGNAL;
  }

  switch (request.signal) {
    case CAN_SIGNAL_POWER:
    case CAN_SIGNAL_AC:
    case CAN_SIGNAL_AUTO:
    case CAN_SIGNAL_INTAKE:
      return request.value <= 1 ? CAN_ERROR_NONE : CAN_ERROR_VALUE_OUT_OF_RANGE;

    case CAN_SIGNAL_FAN_SPEED:
      return isValidFanSpeed(request.value) ? CAN_ERROR_NONE : CAN_ERROR_VALUE_OUT_OF_RANGE;

    case CAN_SIGNAL_TEMPERATURE:
    case CAN_SIGNAL_PASSENGER_TEMPERATURE:
      return isValidTemperature(request.value) ? CAN_ERROR_NONE : CAN_ERROR_VALUE_OUT_OF_RANGE;

    case CAN_SIGNAL_MODE:
      return isValidWindMode(request.value) ? CAN_ERROR_NONE : CAN_ERROR_VALUE_OUT_OF_RANGE;

    case CAN_SIGNAL_SCREEN_MODE:
      return isValidScreenMode(request.value) ? CAN_ERROR_NONE : CAN_ERROR_VALUE_OUT_OF_RANGE;

    case CAN_SIGNAL_MEDIA:
    case CAN_SIGNAL_MAP:
    case CAN_SIGNAL_MUTE:
    case CAN_SIGNAL_NAV:
    case CAN_SIGNAL_RADIO_MODE:
      return request.value <= 1 ? CAN_ERROR_NONE : CAN_ERROR_VALUE_OUT_OF_RANGE;

    case CAN_SIGNAL_VOLUME:
      return isValidVolume(request.value) ? CAN_ERROR_NONE : CAN_ERROR_VALUE_OUT_OF_RANGE;

    case CAN_SIGNAL_RADIO_TUNE:
      return isValidRadioTune(request.value) ? CAN_ERROR_NONE : CAN_ERROR_VALUE_OUT_OF_RANGE;

    default:
      return CAN_ERROR_UNSUPPORTED_SIGNAL;
  }
}

uint8_t canApplyReadRequest(const SystemState& state, const CanPayload& request, uint8_t& value) {
  if (canValidateReadRequest(request) != CAN_ERROR_NONE) {
    value = 0;
    return 0;
  }

  return canSignalValueFromState(state, request.signal, value);
}

uint8_t canValidateReadRequest(const CanPayload& request) {
  return isKnownSignal(request.signal) ? CAN_ERROR_NONE : CAN_ERROR_UNSUPPORTED_SIGNAL;
}

uint8_t canProcessControlRequest(SystemState& state, const CanPayload& request, CanPayload& response) {
  if (!canValidateChecksum(request)) {
    response = canMakeFailResponse(request, CAN_SERVICE_WRITE_RESPONSE, CAN_ERROR_CHECKSUM);
    return 0;
  }

  if (request.service == CAN_SERVICE_WRITE_REQUEST) {
    uint8_t errorCode = canValidateWriteRequest(request);
    if (errorCode != CAN_ERROR_NONE) {
      response = canMakeFailResponse(request, CAN_SERVICE_WRITE_RESPONSE, errorCode);
      return 0;
    }

    canApplyWriteRequest(state, request);
    response = canMakePayload(CAN_SERVICE_WRITE_RESPONSE, CAN_RESULT_SUCCESS, request.signal, request.value, CAN_ERROR_NONE, request.counter);
    return 1;
  }

  if (request.service == CAN_SERVICE_READ_REQUEST) {
    uint8_t value = 0;
    uint8_t errorCode = canValidateReadRequest(request);
    if (errorCode != CAN_ERROR_NONE) {
      response = canMakeFailResponse(request, CAN_SERVICE_READ_RESPONSE, errorCode);
      return 0;
    }

    canApplyReadRequest(state, request, value);
    response = canMakePayload(CAN_SERVICE_READ_RESPONSE, CAN_RESULT_SUCCESS, request.signal, value, CAN_ERROR_NONE, request.counter);
    return 1;
  }

  response = canMakeFailResponse(request, CAN_SERVICE_WRITE_RESPONSE, CAN_ERROR_UNSUPPORTED_SERVICE);
  return 0;
}

CanPayload canMakeStatusPayload(const SystemState& state, uint8_t signal, uint8_t counter) {
  uint8_t value = 0;
  uint8_t success = canSignalValueFromState(state, signal, value);

  return canMakePayload(CAN_SERVICE_READ_RESPONSE, success ? CAN_RESULT_NORMAL : CAN_RESULT_FAIL, signal, value, 0, counter);
}
