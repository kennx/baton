#pragma once

#include "DeviceProfile.h"
#include <vector>
#include <string>

class SignalStorage {
public:
  static constexpr int MAX_PROFILES = 20;
  static constexpr const char* INDEX_FILE = "/profiles.json";

  bool begin();
  bool loadIndex();
  bool saveIndex();

  bool addProfile(const DeviceProfile& profile);
  bool removeProfile(int index);
  DeviceProfile getProfile(int index) const;
  std::vector<DeviceProfile> getAllProfiles() const;
  bool clearAll();
  int getCount() const;

  // JSON utils
  static std::string indexToJson(const std::vector<DeviceProfile>& profiles);
  static std::vector<DeviceProfile> jsonToIndex(const std::string& jsonStr);

private:
  std::vector<DeviceProfile> profiles_;
  bool fsReady_ = false;
};
