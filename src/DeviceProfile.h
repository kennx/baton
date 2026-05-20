#pragma once

#include <string>
#include <vector>

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
