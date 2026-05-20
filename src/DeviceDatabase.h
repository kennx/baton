#pragma once

#include "DeviceProfile.h"
#include <vector>

class DeviceDatabase {
public:
  static std::vector<DeviceProfile> getAllProfiles();
  static DeviceProfile getProfile(const std::string& brand);
};
