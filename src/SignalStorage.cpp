#include "SignalStorage.h"
#include <ArduinoJson.h>

#ifndef UNIT_TEST
#include <LittleFS.h>
#else
#include <fstream>
#include <sstream>
#endif

bool SignalStorage::begin() {
#ifndef UNIT_TEST
  if (!LittleFS.begin(true)) {
    return false;
  }
#endif
  fsReady_ = true;
  loadIndex();
  return true;
}

bool SignalStorage::loadIndex() {
  if (!fsReady_) return false;
  
  std::string content;
#ifndef UNIT_TEST
  if (!LittleFS.exists(INDEX_FILE)) {
    saveIndex();
    return true;
  }
  File file = LittleFS.open(INDEX_FILE, "r");
  if (!file) return false;
  content = file.readString().c_str();
  file.close();
#else
  std::ifstream file("test_profiles.json");
  if (!file.is_open()) return false;
  std::stringstream buffer;
  buffer << file.rdbuf();
  content = buffer.str();
#endif

  profiles_ = jsonToIndex(content);
  return true;
}

bool SignalStorage::saveIndex() {
  if (!fsReady_) return false;

  std::string content = indexToJson(profiles_);
  
#ifndef UNIT_TEST
  File file = LittleFS.open(INDEX_FILE, "w");
  if (!file) return false;
  file.print(content.c_str());
  file.close();
#else
  std::ofstream file("test_profiles.json");
  if (!file.is_open()) return false;
  file << content;
#endif

  return true;
}

bool SignalStorage::addProfile(const DeviceProfile& profile) {
  if (profiles_.size() >= MAX_PROFILES) return false;
  profiles_.push_back(profile);
  return saveIndex();
}

bool SignalStorage::removeProfile(int index) {
  if (index < 0 || index >= static_cast<int>(profiles_.size())) return false;
  profiles_.erase(profiles_.begin() + index);
  return saveIndex();
}

DeviceProfile SignalStorage::getProfile(int index) const {
  if (index < 0 || index >= static_cast<int>(profiles_.size())) return DeviceProfile();
  return profiles_[index];
}

std::vector<DeviceProfile> SignalStorage::getAllProfiles() const {
  return profiles_;
}

bool SignalStorage::clearAll() {
  profiles_.clear();
  return saveIndex();
}

int SignalStorage::getCount() const {
  return profiles_.size();
}

std::string SignalStorage::indexToJson(const std::vector<DeviceProfile>& profiles) {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (const auto& p : profiles) {
    JsonObject obj = arr.add<JsonObject>();
    obj["brand"] = p.brand;
    obj["protocol"] = p.protocol;
    obj["address"] = p.address;
    
    JsonArray cmds = obj["commands"].to<JsonArray>();
    for (const auto& c : p.commands) {
      JsonObject cmdObj = cmds.add<JsonObject>();
      cmdObj["name"] = c.name;
      cmdObj["hexCode"] = c.hexCode;
    }
  }

  std::string output;
  serializeJson(doc, output);
  return output;
}

std::vector<DeviceProfile> SignalStorage::jsonToIndex(const std::string& jsonStr) {
  std::vector<DeviceProfile> profiles;
  if (jsonStr.empty()) return profiles;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonStr);
  if (error) return profiles;

  JsonArray arr = doc.as<JsonArray>();
  for (JsonObject obj : arr) {
    DeviceProfile p;
    p.brand = obj["brand"] | "";
    p.protocol = obj["protocol"] | "";
    p.address = obj["address"] | "";
    
    JsonArray cmds = obj["commands"];
    for (JsonObject cmdObj : cmds) {
      DeviceCommand c;
      c.name = cmdObj["name"] | "";
      c.hexCode = cmdObj["hexCode"] | "";
      p.commands.push_back(c);
    }
    profiles.push_back(p);
  }

  return profiles;
}
