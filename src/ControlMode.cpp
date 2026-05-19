#include "ControlMode.h"
namespace ControlMode {
  void enter(UIScreen& ui, SignalStorage& storage) {}
  void update(UIScreen& ui, SignalStorage& storage, IRController& ir) {}
  void onShortPress(UIScreen& ui, SignalStorage& storage) {}
  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) { return true; }
}
