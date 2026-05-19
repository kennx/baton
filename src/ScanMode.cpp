#include "ScanMode.h"
namespace ScanMode {
  void enter(UIScreen& ui, SignalStorage& storage, IRController& ir) {}
  void update(UIScreen& ui, SignalStorage& storage, IRController& ir) {}
  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {}
  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) { return true; }
}
