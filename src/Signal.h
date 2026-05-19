#pragma once

#include <cstdint>
#include <vector>
#include <string>

struct Signal {
  int id = -1;
  std::string name;
  std::string category;
  std::string protocol;
  std::string address;
  std::string command;
  std::vector<uint16_t> rawData;
  size_t rawLength = 0;
  std::string createdAt;

  bool operator==(const Signal& other) const {
    return protocol == other.protocol &&
           address == other.address &&
           command == other.command;
  }
};
