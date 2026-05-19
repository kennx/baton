#pragma once

#include "SignalData.h"
#include <vector>
#include <string>

class SignalStorage {
public:
  static constexpr int MAX_SIGNALS = 50;
  static constexpr const char* INDEX_FILE = "/signals.json";
  static constexpr const char* RAW_PREFIX = "/signal_";

  bool begin();
  bool loadIndex();
  bool saveIndex();

  bool addSignal(const Signal& signal);
  bool removeSignal(int id);
  Signal getSignal(int id) const;
  std::vector<Signal> getAllSignals() const;
  std::vector<Signal> getSignalsByCategory(const std::string& category) const;
  bool isDuplicate(const std::string& protocol, const std::string& address, const std::string& command) const;
  bool clearAll();
  int getCount() const;
  std::string generateNextName() const;

  // 静态 JSON 转换（支持 native 测试）
  static std::string signalToJson(const Signal& signal);
  static Signal jsonToSignal(const std::string& jsonStr);
  static std::string indexToJson(const std::vector<Signal>& signals);
  static std::vector<Signal> jsonToIndex(const std::string& jsonStr);

private:
  std::vector<Signal> signals_;
  bool fsReady_ = false;

  std::string rawFilePath(int id) const;
  bool saveRawData(int id, const std::vector<uint16_t>& data);
  bool loadRawData(int id, std::vector<uint16_t>& data) const;
  bool removeRawFile(int id);
};
