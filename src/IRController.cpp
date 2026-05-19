#include "IRController.h"

bool IRController::begin() {
  irsend_.begin();
  irrecv_.enableIRIn();
  return true;
}

void IRController::enableReceive() {
  irrecv_.enableIRIn();
}

void IRController::disableReceive() {
  irrecv_.disableIRIn();
}

bool IRController::hasReceived() {
  return irrecv_.decode(&results_);
}

Signal IRController::decodeSignal() {
  Signal s;
  s.protocol = decodeTypeToString(results_.decode_type);
  s.address = uint64ToHex(results_.address);
  s.command = uint64ToHex(results_.command);
  s.rawLength = results_.rawlen;

  // 提取原始时长数组（跳过第一个引导码元素）
  for (uint16_t i = 1; i < results_.rawlen; i++) {
    s.rawData.push_back(results_.rawbuf[i] * RAWTICK);
  }

  irrecv_.resume();
  return s;
}

bool IRController::sendSignal(const Signal& signal) {
  // 尝试通过协议名发射
  if (signal.protocol == "NEC" || signal.protocol == "NECX" || signal.protocol == "NEC42") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    uint32_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    irsend_.sendNEC(addr, cmd, 32);
    return true;
  } else if (signal.protocol == "SONY") {
    uint32_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    irsend_.sendSony(cmd, 12);
    return true;
  } else if (signal.protocol == "SAMSUNG" || signal.protocol == "SAMSUNG36" || signal.protocol == "SAMSUNGAC") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    uint32_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    irsend_.sendSAMSUNG(addr, cmd);
    return true;
  } else if (signal.protocol == "LG" || signal.protocol == "LG2") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    irsend_.sendLG(addr, 32);
    return true;
  } else if (signal.protocol == "RC5" || signal.protocol == "RC5X") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    uint32_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    irsend_.sendRC5(addr, cmd);
    return true;
  } else if (signal.protocol == "RC6" || signal.protocol == "RCMM") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    uint32_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    irsend_.sendRC6(addr, cmd);
    return true;
  } else if (signal.protocol == "PANASONIC" || signal.protocol == "PANASONIC_AC") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    uint32_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    irsend_.sendPanasonic64(addr, cmd);
    return true;
  } else if (signal.protocol == "SHARP") {
    uint32_t addr = strtoul(signal.address.c_str(), nullptr, 16);
    uint32_t cmd = strtoul(signal.command.c_str(), nullptr, 16);
    irsend_.sendSharpRaw(addr, cmd);
    return true;
  }
  // 未知协议 fallback 到 raw 发射
  return sendRaw(signal.rawData);
}

bool IRController::sendRaw(const std::vector<uint16_t>& rawData) {
  if (rawData.empty()) return false;
  irsend_.sendRaw(rawData.data(), rawData.size(), 38);  // 38kHz 载波
  return true;
}

std::string IRController::decodeTypeToString(decode_type_t type) {
  String name = typeToString(type);
  return std::string(name.c_str());
}

std::string IRController::uint64ToHex(uint64_t value) {
  if (value == 0) return "0x0";
  char buf[32];
  snprintf(buf, sizeof(buf), "0x%llX", (unsigned long long)value);
  return std::string(buf);
}
