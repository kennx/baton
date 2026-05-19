#include "IRController.h"
#include <M5Unified.h>
#include <cstring>

bool IRController::begin() {
  irsend_.begin();
  if (!setupRmtRx()) return false;
  return true;
}

// ------------------------------------------------------------------
// RMT RX setup
// ------------------------------------------------------------------
bool IRController::setupRmtRx() {
  rmt_config_t config = RMT_DEFAULT_CONFIG_RX(
      static_cast<gpio_num_t>(IR_RECEIVE_PIN), rxChannel_);
  config.clk_div = 80;                       // 1 us tick
  config.rx_config.idle_threshold = 20000;   // 20 ms idle = end of message
  config.rx_config.filter_ticks_thresh = 100; // 100 us filter
  config.rx_config.filter_en = true;

  if (rmt_config(&config) != ESP_OK) return false;

  size_t rxBufSize = MAX_RX_SYMBOLS * sizeof(rmt_item32_t);
  if (rmt_driver_install(rxChannel_, rxBufSize, 0) != ESP_OK) return false;

  if (rmt_get_ringbuf_handle(rxChannel_, &rxRingbuf_) != ESP_OK) return false;

  return true;
}

void IRController::enableReceive() {
  // Disable speaker amp to avoid IR RX interference (M5StickS3 requirement)
  if (M5.Speaker.isEnabled()) {
    M5.Speaker.end();
  }

  // Clear any pending data
  if (pendingRxItems_) {
    vRingbufferReturnItem(rxRingbuf_, pendingRxItems_);
    pendingRxItems_ = nullptr;
    pendingRxCount_ = 0;
  }

  rmt_rx_memory_reset(rxChannel_);
  rmt_rx_start(rxChannel_, true);
  rxEnabled_ = true;
}

void IRController::disableReceive() {
  rmt_rx_stop(rxChannel_);
  rxEnabled_ = false;

  if (pendingRxItems_) {
    vRingbufferReturnItem(rxRingbuf_, pendingRxItems_);
    pendingRxItems_ = nullptr;
    pendingRxCount_ = 0;
  }
}

bool IRController::hasReceived() {
  if (!rxEnabled_ || !rxRingbuf_) return false;

  // If we already have pending data from a previous hasReceived() call,
  // return true so decodeSignal() can process it.
  if (pendingRxItems_ != nullptr) return true;

  // Try to receive new data (non-blocking)
  size_t rxSize = 0;
  pendingRxItems_ = static_cast<rmt_item32_t*>(
      xRingbufferReceive(rxRingbuf_, &rxSize, 0));

  if (pendingRxItems_) {
    pendingRxCount_ = rxSize / sizeof(rmt_item32_t);
    return true;
  }

  return false;
}

Signal IRController::decodeSignal() {
  Signal s;

  if (!rxEnabled_ || !rxRingbuf_) return s;

  // If hasReceived() was called, use the pending items.
  // Otherwise try to read directly.
  rmt_item32_t* items = pendingRxItems_;
  size_t count = pendingRxCount_;

  if (!items) {
    size_t rxSize = 0;
    items = static_cast<rmt_item32_t*>(
        xRingbufferReceive(rxRingbuf_, &rxSize, 0));
    if (!items) return s;
    count = rxSize / sizeof(rmt_item32_t);
  }

  // Convert RMT items to raw pulse durations
  itemsToRawData(items, count, s.rawData);
  s.rawLength = s.rawData.size();

  // Try NEC decode
  uint16_t addr = 0;
  uint8_t cmd = 0;
  bool isRepeat = false;

  if (decodeNEC(items, count, &addr, &cmd, &isRepeat)) {
    s.protocol = "NEC";
    s.address = uint16ToHex(addr);
    s.command = uint8ToHex(cmd);
  } else {
    s.protocol = "UNKNOWN";
    s.address = "0x0";
    s.command = "0x0";
  }

  // Release items back to ringbuffer
  vRingbufferReturnItem(rxRingbuf_, items);
  pendingRxItems_ = nullptr;
  pendingRxCount_ = 0;

  // Re-arm receiver for next frame
  rmt_rx_memory_reset(rxChannel_);
  rmt_rx_start(rxChannel_, true);

  return s;
}

// ------------------------------------------------------------------
// NEC encoding / decoding
// ------------------------------------------------------------------
uint32_t IRController::necRaw(uint16_t address, uint8_t command) {
  uint16_t necAddr;
  if (address <= 0x00FF) {
    uint8_t addr8 = address & 0xFF;
    necAddr = (static_cast<uint16_t>(~addr8) << 8) | addr8;
  } else {
    necAddr = address;
  }
  uint32_t raw = 0;
  raw |= static_cast<uint32_t>(necAddr);
  raw |= static_cast<uint32_t>(command) << 16;
  raw |= static_cast<uint32_t>(~command) << 24;
  return raw;
}

bool IRController::decodeNEC(rmt_item32_t* items, size_t count,
                             uint16_t* outAddr, uint8_t* outCmd, bool* outRepeat) {
  *outAddr = 0;
  *outCmd = 0;
  *outRepeat = false;

  if (count < 2) return false;

  // IR receiver is active-low:
  //   IR mark (carrier present)  -> receiver outputs LOW  (level = 0)
  //   IR space (no carrier)      -> receiver outputs HIGH (level = 1)
  uint32_t headerMark  = items[0].duration0;
  uint32_t headerSpace = items[0].duration1;

  // Repeat frame: ~9ms mark + ~2.25ms space
  if (headerMark > 8000 && headerSpace > 2000 && headerSpace < 3000) {
    *outRepeat = true;
    return false;
  }

  // Standard NEC header: ~9ms mark + ~4.5ms space
  if (!(headerMark > 8000 && headerSpace > 4000)) {
    return false;
  }

  // Need header + 32 bits + ending mark (at least 34 symbols)
  if (count < 34) return false;

  uint32_t raw = 0;
  for (int i = 0; i < 32; i++) {
    uint32_t mark  = items[i + 1].duration0;
    uint32_t space = items[i + 1].duration1;

    // mark should be ~560us (accept 300-800us tolerance)
    if (mark < 300 || mark > 800) return false;

    // space > 1000us means logic 1 (~1690us), else logic 0 (~560us)
    if (space > 1000) {
      raw |= (1UL << i);
    }
  }

  // Verify command inverse
  uint8_t cmd = (raw >> 16) & 0xFF;
  uint8_t cmdInv = (raw >> 24) & 0xFF;
  if ((cmd ^ cmdInv) != 0xFF) return false;

  *outAddr = raw & 0xFFFF;
  *outCmd = cmd;
  return true;
}

// ------------------------------------------------------------------
// Raw data conversion
// ------------------------------------------------------------------
void IRController::itemsToRawData(rmt_item32_t* items, size_t count,
                                  std::vector<uint16_t>& rawData) {
  for (size_t i = 0; i < count; i++) {
    if (items[i].duration0 > 0) {
      rawData.push_back(items[i].duration0);
    }
    if (items[i].duration1 > 0) {
      rawData.push_back(items[i].duration1);
    }
  }
}

// ------------------------------------------------------------------
// Transmission (IRremoteESP8266 IRsend)
// ------------------------------------------------------------------
bool IRController::sendSignal(const Signal& signal) {
  // If rawData available, use it directly
  if (!signal.rawData.empty()) {
    return sendRaw(signal.rawData);
  }

  // Otherwise encode by protocol
  if (signal.protocol == "NEC" || signal.protocol == "NECX" || signal.protocol == "NEC42") {
    uint16_t addr = static_cast<uint16_t>(strtoul(signal.address.c_str(), nullptr, 16));
    uint16_t cmd = static_cast<uint16_t>(strtoul(signal.command.c_str(), nullptr, 16));
    uint32_t data = irsend_.encodeNEC(addr, cmd);
    irsend_.sendNEC(data, 32);
    return true;
  } else if (signal.protocol == "SONY") {
    uint64_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    irsend_.sendSony(cmd, 12);
    return true;
  } else if (signal.protocol == "SAMSUNG" || signal.protocol == "SAMSUNG36" || signal.protocol == "SAMSUNGAC") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    uint32_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    uint64_t data = (static_cast<uint64_t>(addr) << 32) | cmd;
    irsend_.sendSAMSUNG(data, 32);
    return true;
  } else if (signal.protocol == "LG" || signal.protocol == "LG2") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    irsend_.sendLG(addr, 32);
    return true;
  } else if (signal.protocol == "RC5" || signal.protocol == "RC5X") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    uint32_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    uint64_t data = (static_cast<uint64_t>(addr) << 16) | cmd;
    irsend_.sendRC5(data, 14);
    return true;
  } else if (signal.protocol == "RC6" || signal.protocol == "RCMM") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    uint32_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    uint64_t data = (static_cast<uint64_t>(addr) << 16) | cmd;
    irsend_.sendRC6(data, 20);
    return true;
  } else if (signal.protocol == "PANASONIC" || signal.protocol == "PANASONIC_AC") {
    uint16_t addr = static_cast<uint16_t>(strtoul(signal.address.c_str(), nullptr, 16));
    uint32_t cmd = static_cast<uint32_t>(strtoul(signal.command.c_str(), nullptr, 16));
    irsend_.sendPanasonic(addr, cmd, 48);
    return true;
  } else if (signal.protocol == "SHARP") {
    uint16_t addr = static_cast<uint16_t>(strtoul(signal.address.c_str(), nullptr, 16));
    uint16_t cmd = static_cast<uint16_t>(strtoul(signal.command.c_str(), nullptr, 16));
    irsend_.sendSharp(addr, cmd);
    return true;
  }

  return false;
}

bool IRController::sendRaw(const std::vector<uint16_t>& rawData) {
  if (rawData.empty()) return false;
  irsend_.sendRaw(rawData.data(), rawData.size(), 38);
  return true;
}

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------
std::string IRController::uint16ToHex(uint16_t value) {
  if (value == 0) return "0x0";
  char buf[16];
  snprintf(buf, sizeof(buf), "0x%04X", value);
  return std::string(buf);
}

std::string IRController::uint8ToHex(uint8_t value) {
  if (value == 0) return "0x0";
  char buf[16];
  snprintf(buf, sizeof(buf), "0x%02X", value);
  return std::string(buf);
}
