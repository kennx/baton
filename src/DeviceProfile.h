#pragma once

#include <string>
#include <vector>

// Dynamic structs (used for saved profiles in SignalStorage)
struct DeviceCommand {
  std::string name;
  std::string hexCode;
};

struct DeviceProfile {
  std::string brand;
  std::string protocol;
  std::string address;
  std::vector<DeviceCommand> commands;
};

// Static structs (stored in PROGMEM Flash, used by DeviceDatabase)
struct DeviceCommandStatic {
  const char* name;
  const char* hexCode;
};

struct DeviceProfileStatic {
  const char* brand;
  const char* protocol;
  const char* address;
  const DeviceCommandStatic* commands;
  size_t commandCount;
};

struct DeviceCategoryStatic {
  const char* name;
  const DeviceProfileStatic* profiles;
  size_t profileCount;
};
