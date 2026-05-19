#pragma once

#include "SignalData.h"
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

class IRController {
public:
  static constexpr uint16_t IR_SEND_PIN = 46;
  static constexpr uint16_t IR_RECEIVE_PIN = 42;
  static constexpr uint16_t RECEIVE_BUFFER_SIZE = 1024;
  static constexpr uint8_t RECEIVE_TIMEOUT_MS = 50;

  bool begin();
  void enableReceive();
  void disableReceive();
  bool hasReceived();
  Signal decodeSignal();
  bool sendSignal(const Signal& signal);
  bool sendRaw(const std::vector<uint16_t>& rawData);

private:
  IRrecv irrecv_{IR_RECEIVE_PIN, RECEIVE_BUFFER_SIZE, RECEIVE_TIMEOUT_MS, true};
  IRsend irsend_{IR_SEND_PIN};
  decode_results results_;

  std::string decodeTypeToString(decode_type_t type);
  std::string uint64ToHex(uint64_t value);
};
