#include "CommandParser.h"
#include "CanProtocol.h"
#include "GDS.h"

static String nextToken(String& line) {
  line.trim();
  int spaceIndex = line.indexOf(' ');

  if (spaceIndex < 0) {
    String token = line;
    line = "";
    token.trim();
    return token;
  }

  String token = line.substring(0, spaceIndex);
  line = line.substring(spaceIndex + 1);
  token.trim();
  return token;
}

static uint8_t parseOnOffValue(const String& token, uint8_t& value) {
  if (token == "on" || token == "ready" || token == "1") {
    value = 1;
    return 1;
  }

  if (token == "off" || token == "dev" || token == "0") {
    value = 0;
    return 1;
  }

  return 0;
}

static uint8_t parseDecimalByte(const String& token, uint8_t minValue, uint8_t maxValue, uint8_t& value) {
  if (token.length() == 0) {
    return 0;
  }

  for (uint16_t i = 0; i < token.length(); i++) {
    if (!isDigit(token.charAt(i))) {
      return 0;
    }
  }

  int parsed = token.toInt();
  if (parsed < minValue || parsed > maxValue) {
    return 0;
  }

  value = (uint8_t)parsed;
  return 1;
}

static HuCommand makeWrite(uint8_t signal, uint8_t value) {
  HuCommand command;
  command.valid = 1;
  command.service = CAN_SERVICE_WRITE_REQUEST;
  command.signal = signal;
  command.value = value;
  return command;
}

static HuCommand makeRead(uint8_t signal) {
  HuCommand command;
  command.valid = 1;
  command.service = CAN_SERVICE_READ_REQUEST;
  command.signal = signal;
  command.value = 0;
  return command;
}

static HuCommand invalidCommand() {
  HuCommand command;
  command.valid = 0;
  command.service = 0;
  command.signal = 0;
  command.value = 0;
  return command;
}

HuCommand parseHuCommand(String line) {
  line.trim();
  line.toLowerCase();

  String command = nextToken(line);
  String arg = nextToken(line);

  if (command == "help" || command == "?") {
    printHuHelp();
    return invalidCommand();
  }

  if (command == "read") {
    if (arg == "power") return makeRead(CAN_SIGNAL_POWER);
    if (arg == "fan") return makeRead(CAN_SIGNAL_FAN_SPEED);
    if (arg == "temp") return makeRead(CAN_SIGNAL_TEMPERATURE);
    if (arg == "wind") return makeRead(CAN_SIGNAL_MODE);
    if (arg == "ac") return makeRead(CAN_SIGNAL_AC);
    if (arg == "auto") return makeRead(CAN_SIGNAL_AUTO);
    if (arg == "screen") return makeRead(CAN_SIGNAL_SCREEN_MODE);
    if (arg == "media") return makeRead(CAN_SIGNAL_MEDIA);
    if (arg == "volume") return makeRead(CAN_SIGNAL_VOLUME);
    if (arg == "map") return makeRead(CAN_SIGNAL_MAP);
    return invalidCommand();
  }

  if (command == "fan") {
    uint8_t value = 0;
    if (!parseDecimalByte(arg, 0, GDS_FAN_SPEED_MAX, value)) return invalidCommand();
    return makeWrite(CAN_SIGNAL_FAN_SPEED, value);
  }

  if (command == "temp") {
    uint8_t value = 0;
    if (!parseDecimalByte(arg, GDS_TEMP_MIN, GDS_TEMP_MAX, value)) return invalidCommand();
    return makeWrite(CAN_SIGNAL_TEMPERATURE, value);
  }

  if (command == "volume" || command == "vol") {
    uint8_t value = 0;
    if (!parseDecimalByte(arg, 0, GDS_VOLUME_MAX, value)) return invalidCommand();
    return makeWrite(CAN_SIGNAL_VOLUME, value);
  }

  if (command == "screen") {
    if (arg == "datc") return makeWrite(CAN_SIGNAL_SCREEN_MODE, 0);
    if (arg == "info") return makeWrite(CAN_SIGNAL_SCREEN_MODE, 1);
    return invalidCommand();
  }

  if (command == "wind") {
    if (arg == "face") return makeWrite(CAN_SIGNAL_MODE, 0);
    if (arg == "foot") return makeWrite(CAN_SIGNAL_MODE, 1);
    if (arg == "def") return makeWrite(CAN_SIGNAL_MODE, 2);
    if (arg == "mix") return makeWrite(CAN_SIGNAL_MODE, 3);
    return invalidCommand();
  }

  uint8_t value = 0;
  if (command == "power" && parseOnOffValue(arg, value)) return makeWrite(CAN_SIGNAL_POWER, value);
  if (command == "ac" && parseOnOffValue(arg, value)) return makeWrite(CAN_SIGNAL_AC, value);
  if (command == "auto" && parseOnOffValue(arg, value)) return makeWrite(CAN_SIGNAL_AUTO, value);
  if (command == "media" && parseOnOffValue(arg, value)) return makeWrite(CAN_SIGNAL_MEDIA, value);
  if (command == "map" && parseOnOffValue(arg, value)) return makeWrite(CAN_SIGNAL_MAP, value);

  return invalidCommand();
}

void printHuHelp() {
  Serial.println();
  Serial.println("HU Serial Commands:");
  Serial.println("  fan 0..8");
  Serial.println("  temp 18..30");
  Serial.println("  volume 0..30");
  Serial.println("  screen datc|info");
  Serial.println("  wind face|foot|def|mix");
  Serial.println("  power on|off");
  Serial.println("  ac on|off");
  Serial.println("  auto on|off");
  Serial.println("  media ready|dev");
  Serial.println("  map ready|dev");
  Serial.println("  read fan|temp|screen|media|volume|map");
  Serial.println();
}
