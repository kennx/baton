#pragma once
#include "UIScreen.h"
#include "SignalStorage.h"
#include "IRController.h"

namespace BruteForceMode {
  void enter(UIScreen& ui, SignalStorage& storage, IRController& ir);
  void update(UIScreen& ui, SignalStorage& storage, IRController& ir);
  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
}
