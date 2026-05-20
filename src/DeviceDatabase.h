#pragma once

#include "DeviceProfile.h"
#include <vector>
#include <cstddef>

class DeviceDatabase {
public:
  // Category navigation
  static size_t getCategoryCount();
  static const char* getCategoryName(size_t categoryIndex);
  
  // Profile navigation within a category
  static size_t getProfileCount(size_t categoryIndex);
  static const DeviceProfileStatic* getProfile(size_t categoryIndex, size_t profileIndex);
  
  // Convert a static profile to a dynamic one (for saving to storage)
  static DeviceProfile toDynamic(const DeviceProfileStatic* profile);
};
