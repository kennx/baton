#include "IRController.h"
#include <M5Unified.h>

bool IRController::begin() {
  irsend_.begin();
  return true;
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
