#pragma once

#include "SignalData.h"
#include <IRsend.h>

class IRController {
public:
  static constexpr uint16_t IR_SEND_PIN = 46;

  bool begin();

  // TX (via IRremoteESP8266 IRsend)
  bool sendSignal(const Signal& signal);
  bool sendRaw(const std::vector<uint16_t>& rawData);

private:
  // IRsend TX
  IRsend irsend_{IR_SEND_PIN};
};
