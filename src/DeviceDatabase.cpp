#include "DeviceDatabase.h"

std::vector<DeviceProfile> DeviceDatabase::getAllProfiles() {
  std::vector<DeviceProfile> profiles;

  // Samsung
  profiles.push_back({
    "Samsung TV",
    "SAMSUNG",
    "0x0707",
    {
      {"Power", "0x02"},
      {"Vol+", "0x07"},
      {"Vol-", "0x0B"},
      {"Mute", "0x0F"},
      {"Ch+", "0x12"},
      {"Ch-", "0x10"}
    }
  });

  // LG
  profiles.push_back({
    "LG TV",
    "LG",
    "0x20DF",
    {
      {"Power", "0x10EF"}, // 0x10 is standard LG Power command, but LG uses 32-bit typically. We use just 0x10 here.
      // Wait, in previous BruteForceMode, LG Addr was 0x20DF, commands were 0x10, 0x14, etc.
      {"Power", "0x10"},
      {"Vol+", "0x40"},
      {"Vol-", "0xC0"},
      {"Mute", "0x90"},
      {"Ch+", "0x00"},
      {"Ch-", "0x80"}
    }
  });

  // Sony
  profiles.push_back({
    "Sony TV",
    "SONY",
    "0x0001",
    {
      {"Power", "0xA90"},
      {"Vol+", "0x490"},
      {"Vol-", "0xC90"},
      {"Mute", "0x290"},
      {"Ch+", "0x090"},
      {"Ch-", "0x890"}
    }
  });

  // Panasonic
  profiles.push_back({
    "Panasonic TV",
    "PANASONIC",
    "0x4004",
    {
      {"Power", "0x0100"}, // Typical Panasonic Power
      {"Vol+", "0x0400"},
      {"Vol-", "0x0C00"},
      {"Mute", "0x0D00"},
      {"Ch+", "0x0200"},
      {"Ch-", "0x0A00"}
    }
  });

  // Generic NEC
  profiles.push_back({
    "Generic NEC",
    "NEC",
    "0x00FF",
    {
      {"Power", "0x45"},
      {"Vol+", "0x46"},
      {"Vol-", "0x15"},
      {"Mute", "0x47"},
      {"Ch+", "0x44"},
      {"Ch-", "0x43"}
    }
  });

  return profiles;
}

DeviceProfile DeviceDatabase::getProfile(const std::string& brand) {
  auto profiles = getAllProfiles();
  for (const auto& p : profiles) {
    if (p.brand == brand) return p;
  }
  return profiles.empty() ? DeviceProfile() : profiles.front();
}
