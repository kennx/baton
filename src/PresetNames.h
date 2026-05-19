#pragma once
#include <vector>
#include <string>

namespace PresetNames {

  const std::vector<std::string> TV = {
    "Power", "Vol+", "Vol-", "CH+", "CH-", "Mute",
    "Menu", "Up", "Down", "Left", "Right", "OK", "Back"
  };
  const std::vector<std::string> AC = {
    "Power", "Cool", "Heat", "Dry", "Auto", "Temp+",
    "Temp-", "Fan", "Swing", "Timer"
  };
  const std::vector<std::string> FAN = {
    "Power", "Fan", "Osc", "Timer"
  };
  const std::vector<std::string> STB = {
    "Power", "Menu", "Back", "Live", "Up", "Down",
    "Left", "Right", "OK", "Back"
  };
  const std::vector<std::string> CUSTOM = {
    "Custom-01", "Custom-02", "Custom-03", "Custom-04", "Custom-05"
  };

  const std::vector<std::string> CATEGORIES = {
    "TV", "AC", "FAN", "STB", "Custom"
  };

  inline const std::vector<std::string>* getNameList(const std::string& category) {
    if (category == "TV") return &TV;
    if (category == "AC") return &AC;
    if (category == "FAN") return &FAN;
    if (category == "STB") return &STB;
    return &CUSTOM;
  }

} // namespace PresetNames
