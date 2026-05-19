#pragma once
#include "UIScreen.h"
#include "SignalStorage.h"

namespace SignalManager {
  void enter(UIScreen& ui, SignalStorage& storage);
  void update(UIScreen& ui, SignalStorage& storage);
  void onShortPress(UIScreen& ui, SignalStorage& storage);
  bool onLongPress(UIScreen& ui, SignalStorage& storage);
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage);
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage);
}
