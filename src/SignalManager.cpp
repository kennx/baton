#include "SignalManager.h"

namespace {
  enum class MgrSubState { LIST, DELETE_CONFIRM };
  MgrSubState subState = MgrSubState::LIST;

  std::vector<DeviceProfile> profiles;
  int profileIndex = 0;
  int deleteOption = 0;

  void refreshProfiles(SignalStorage& storage) {
    profiles = storage.getAllProfiles();
    if (profileIndex >= static_cast<int>(profiles.size())) {
      profileIndex = 0;
    }
  }

  void drawList(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> items;
    for (const auto& p : profiles) {
      items.push_back(p.brand);
    }
    if (items.empty()) {
      items.push_back("(no devices)");
    }
    ui.drawStatusBar("[B]", storage.getCount(), "[M]MGMT");
    ui.drawMenu(items, profileIndex, "Pick device");
    ui.drawFooter("Short:NXT Long:DEL");
  }

  void drawDeleteConfirm(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> opts = {"Cancel", "Confirm"};
    ui.drawPopup("[!] DEL?", "Del " + profiles[profileIndex].brand + "?\nCannot undo!", opts, deleteOption);
  }
} // namespace

namespace SignalManager {

  void enter(UIScreen& ui, SignalStorage& storage) {
    subState = MgrSubState::LIST;
    profileIndex = 0;
    refreshProfiles(storage);
    drawList(ui, storage);
  }

  void update(UIScreen& ui, SignalStorage& storage) {
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == MgrSubState::LIST) {
      if (!profiles.empty()) {
        profileIndex = (profileIndex + 1) % profiles.size();
      }
      drawList(ui, storage);
    } else if (subState == MgrSubState::DELETE_CONFIRM) {
      deleteOption = (deleteOption + 1) % 2;
      drawDeleteConfirm(ui, storage);
    }
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == MgrSubState::LIST) {
      if (profiles.empty()) return true;
      subState = MgrSubState::DELETE_CONFIRM;
      deleteOption = 0;
      drawDeleteConfirm(ui, storage);
      return false;
    } else if (subState == MgrSubState::DELETE_CONFIRM) {
      if (deleteOption == 1) { // Confirm
        storage.removeProfile(profileIndex);
        refreshProfiles(storage);
      }
      subState = MgrSubState::LIST;
      drawList(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == MgrSubState::LIST) {
      return true; // back to main menu
    } else if (subState == MgrSubState::DELETE_CONFIRM) {
      subState = MgrSubState::LIST;
      drawList(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage) {
    return true;
  }

}
