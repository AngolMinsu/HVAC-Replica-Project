#ifndef HU_COMMAND_PARSER_H
#define HU_COMMAND_PARSER_H

#include <Arduino.h>

struct HuCommand {
  uint8_t valid;
  uint8_t service;
  uint8_t signal;
  uint8_t value;
};

HuCommand parseHuCommand(String line);
void printHuHelp();

#endif
