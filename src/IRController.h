#pragma once

#include "SignalData.h"
#include <driver/rmt.h>
#include <IRsend.h>

class IRController {
public:
  static constexpr uint16_t IR_SEND_PIN = 46;
  static constexpr uint16_t IR_RECEIVE_PIN = 42;
  static constexpr size_t MAX_RX_SYMBOLS = 256;

  bool begin();

  // TX (via IRremoteESP8266 IRsend)
  bool sendSignal(const Signal& signal);
  bool sendRaw(const std::vector<uint16_t>& rawData);

  // RX (via ESP-IDF RMT)
  void enableReceive();
  void disableReceive();
  bool hasReceived();
  Signal decodeSignal();

private:
  // RMT RX
  rmt_channel_t rxChannel_ = RMT_CHANNEL_0;
  RingbufHandle_t rxRingbuf_ = nullptr;
  bool rxEnabled_ = false;
  rmt_item32_t* pendingRxItems_ = nullptr;
  size_t pendingRxCount_ = 0;

  // IRsend TX
  IRsend irsend_{IR_SEND_PIN};

  bool setupRmtRx();

  bool decodeNEC(rmt_item32_t* items, size_t count,
                 uint16_t* outAddr, uint8_t* outCmd, bool* outRepeat);
  void itemsToRawData(rmt_item32_t* items, size_t count,
                      std::vector<uint16_t>& rawData);

  uint32_t necRaw(uint16_t address, uint8_t command);
  std::string uint16ToHex(uint16_t value);
  std::string uint8ToHex(uint8_t value);
};
