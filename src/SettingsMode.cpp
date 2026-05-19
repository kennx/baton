#include "SettingsMode.h"
namespace SettingsMode {
  void enter(UIScreen& ui, SignalStorage& storage) {}
  void update(UIScreen& ui, SignalStorage& storage) {}
  void onShortPress(UIScreen& ui, SignalStorage& storage) {}
  bool onLongPress(UIScreen& ui, SignalStorage& storage) { return true; }
  int getRepeatCount() { return 2; }
  int getBrightness() { return 80; }
  bool getBuzzerEnabled() { return true; }
}
