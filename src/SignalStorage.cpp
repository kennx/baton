#include "SignalStorage.h"

#ifndef UNIT_TEST
#include <LittleFS.h>
#endif

#include <ArduinoJson.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

bool SignalStorage::begin() {
#ifndef UNIT_TEST
  fsReady_ = LittleFS.begin(true);
  if (fsReady_) {
    loadIndex();
  }
  return fsReady_;
#else
  fsReady_ = true;
  return true;
#endif
}

bool SignalStorage::loadIndex() {
#ifndef UNIT_TEST
  if (!LittleFS.exists(INDEX_FILE)) {
    signals_.clear();
    return true;
  }
  File f = LittleFS.open(INDEX_FILE, "r");
  if (!f) return false;
  std::string content = f.readString().c_str();
  f.close();
  signals_ = jsonToIndex(content);
#endif
  return true;
}

bool SignalStorage::saveIndex() {
#ifndef UNIT_TEST
  std::string json = indexToJson(signals_);
  File f = LittleFS.open(INDEX_FILE, "w");
  if (!f) return false;
  f.print(json.c_str());
  f.close();
#endif
  return true;
}

bool SignalStorage::addSignal(const Signal& signal) {
  if (signals_.size() >= static_cast<size_t>(MAX_SIGNALS)) {
    return false;
  }
  Signal s = signal;
  s.id = signals_.empty() ? 0 : signals_.back().id + 1;
  if (s.name.empty()) {
    s.name = generateNextName();
  }
  if (s.category.empty()) {
    s.category = "未分类";
  }

#ifndef UNIT_TEST
  if (!saveRawData(s.id, s.rawData)) {
    return false;
  }
#endif

  signals_.push_back(s);
  return saveIndex();
}

bool SignalStorage::removeSignal(int id) {
  auto it = std::find_if(signals_.begin(), signals_.end(),
                         [id](const Signal& s) { return s.id == id; });
  if (it == signals_.end()) return false;

#ifndef UNIT_TEST
  removeRawFile(id);
#endif

  signals_.erase(it);
  return saveIndex();
}

Signal SignalStorage::getSignal(int id) const {
  auto it = std::find_if(signals_.begin(), signals_.end(),
                         [id](const Signal& s) { return s.id == id; });
  if (it != signals_.end()) {
    Signal result = *it;
#ifndef UNIT_TEST
    loadRawData(id, result.rawData);
    result.rawLength = result.rawData.size();
#endif
    return result;
  }
  return Signal{};
}

std::vector<Signal> SignalStorage::getAllSignals() const {
  return signals_;
}

std::vector<Signal> SignalStorage::getSignalsByCategory(const std::string& category) const {
  std::vector<Signal> result;
  std::copy_if(signals_.begin(), signals_.end(), std::back_inserter(result),
               [&category](const Signal& s) { return s.category == category; });
  return result;
}

bool SignalStorage::isDuplicate(const std::string& protocol,
                                const std::string& address,
                                const std::string& command) const {
  return std::any_of(signals_.begin(), signals_.end(),
                     [&protocol, &address, &command](const Signal& s) {
                       return s.protocol == protocol &&
                              s.address == address &&
                              s.command == command;
                     });
}

bool SignalStorage::clearAll() {
#ifndef UNIT_TEST
  for (const auto& s : signals_) {
    removeRawFile(s.id);
  }
#endif
  signals_.clear();
  return saveIndex();
}

int SignalStorage::getCount() const {
  return static_cast<int>(signals_.size());
}

std::string SignalStorage::generateNextName() const {
  return "Signal-" + std::to_string(signals_.size() + 1);
}

std::string SignalStorage::signalToJson(const Signal& signal) {
  JsonDocument doc;
  doc["id"] = signal.id;
  doc["name"] = signal.name;
  doc["category"] = signal.category;
  doc["protocol"] = signal.protocol;
  doc["address"] = signal.address;
  doc["command"] = signal.command;
  doc["length"] = signal.rawLength;
  doc["created_at"] = signal.createdAt;

  std::string output;
  serializeJson(doc, output);
  return output;
}

Signal SignalStorage::jsonToSignal(const std::string& jsonStr) {
  JsonDocument doc;
  deserializeJson(doc, jsonStr);

  Signal s;
  s.id = doc["id"] | -1;
  s.name = doc["name"] | "";
  s.category = doc["category"] | "";
  s.protocol = doc["protocol"] | "";
  s.address = doc["address"] | "";
  s.command = doc["command"] | "";
  s.rawLength = doc["length"] | 0;
  s.createdAt = doc["created_at"] | "";
  return s;
}

std::string SignalStorage::indexToJson(const std::vector<Signal>& signals) {
  JsonDocument doc;
  doc["version"] = 1;
  doc["count"] = signals.size();

  JsonArray arr = doc["signals"].to<JsonArray>();
  for (const auto& s : signals) {
    JsonObject obj = arr.add<JsonObject>();
    obj["id"] = s.id;
    obj["name"] = s.name;
    obj["category"] = s.category;
    obj["protocol"] = s.protocol;
    obj["address"] = s.address;
    obj["command"] = s.command;
    obj["length"] = s.rawLength;
    obj["created_at"] = s.createdAt;
  }

  std::string output;
  serializeJsonPretty(doc, output);
  return output;
}

std::vector<Signal> SignalStorage::jsonToIndex(const std::string& jsonStr) {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, jsonStr);
  if (err) return {};

  std::vector<Signal> result;
  JsonArray arr = doc["signals"];
  for (JsonObject obj : arr) {
    Signal s;
    s.id = obj["id"] | -1;
    s.name = obj["name"] | "";
    s.category = obj["category"] | "";
    s.protocol = obj["protocol"] | "";
    s.address = obj["address"] | "";
    s.command = obj["command"] | "";
    s.rawLength = obj["length"] | 0;
    s.createdAt = obj["created_at"] | "";
    result.push_back(s);
  }
  return result;
}

#ifndef UNIT_TEST

std::string SignalStorage::rawFilePath(int id) const {
  return std::string(RAW_PREFIX) + std::to_string(id) + ".raw";
}

bool SignalStorage::saveRawData(int id, const std::vector<uint16_t>& data) {
  std::string path = rawFilePath(id);
  File f = LittleFS.open(path.c_str(), "w");
  if (!f) return false;
  f.write(reinterpret_cast<const uint8_t*>(data.data()), data.size() * sizeof(uint16_t));
  f.close();
  return true;
}

bool SignalStorage::loadRawData(int id, std::vector<uint16_t>& data) const {
  std::string path = rawFilePath(id);
  if (!LittleFS.exists(path.c_str())) return false;
  File f = LittleFS.open(path.c_str(), "r");
  if (!f) return false;
  size_t size = f.size();
  data.resize(size / sizeof(uint16_t));
  f.read(reinterpret_cast<uint8_t*>(data.data()), size);
  f.close();
  return true;
}

bool SignalStorage::removeRawFile(int id) {
  std::string path = rawFilePath(id);
  if (LittleFS.exists(path.c_str())) {
    return LittleFS.remove(path.c_str());
  }
  return true;
}

#endif
