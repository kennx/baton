#include "MatchMode.h"
#include <M5Unified.h>

namespace {
  enum class MState { CATEGORY_SELECT, REMOTE_SELECT, TEST_POWER, SAVE_CONFIRM };
  MState mState = MState::CATEGORY_SELECT;

  size_t categoryIndex = 0;
  size_t remoteIndex = 0;

  const DeviceProfileStatic* currentStaticProfile = nullptr;
  DeviceProfile currentProfile;  // dynamic copy for saving
  DeviceCommand powerCmd;

  void drawCategorySelect(UIScreen& ui, SignalStorage& storage) {
    size_t count = DeviceDatabase::getCategoryCount();
    ui.drawStatusBar("[B]", storage.getCount(), "MATCH");
    ui.drawMenu(count, categoryIndex, [](int idx) {
      return std::string(DeviceDatabase::getCategoryName(idx)) + " (" + std::to_string(DeviceDatabase::getProfileCount(idx)) + ")";
    }, "Category");
    ui.drawFooter("NXT        SEL");
  }

  void drawRemoteSelect(UIScreen& ui, SignalStorage& storage) {
    size_t count = DeviceDatabase::getProfileCount(categoryIndex);
    const char* catName = DeviceDatabase::getCategoryName(categoryIndex);
    ui.drawStatusBar("[B]", storage.getCount(), "MATCH");
    ui.drawMenu(count, remoteIndex, [](int idx) {
      const auto* p = DeviceDatabase::getProfile(categoryIndex, idx);
      return p ? std::string(p->brand) : "";
    }, catName);
    ui.drawFooter("NXT        TEST");
  }

  void drawTestPower(UIScreen& ui, SignalStorage& storage, bool success = true) {
    ui.drawStatusBar("[B]", storage.getCount(), "TEST");
    std::string statusMsg = success ? "Test Power" : "Unsupported!";
    ui.drawSignalInfo(currentProfile.brand, currentProfile.protocol, currentProfile.address, powerCmd.hexCode, statusMsg);
    ui.drawFooter("SEND       SAVE");
  }

  void drawSaveConfirm(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "SAVE");
    ui.drawSignalInfo("Save?", currentProfile.brand, "", "", "Hold to confirm");
    ui.drawFooter("BACK       SAVE");
  }

  void selectCurrentProfile() {
    currentStaticProfile = DeviceDatabase::getProfile(categoryIndex, remoteIndex);
    if (!currentStaticProfile) return;
    currentProfile = DeviceDatabase::toDynamic(currentStaticProfile);
    
    // Find power command
    powerCmd = {"Power", "0x00"};
    for (const auto& c : currentProfile.commands) {
      std::string lower = c.name;
      for (auto& ch : lower) ch = tolower(ch);
      if (lower.find("power") != std::string::npos || lower.find("on") != std::string::npos || lower.find("off") != std::string::npos) {
        powerCmd = c;
        break;
      }
    }
    // Fallback: use first command
    if (powerCmd.hexCode == "0x00" && !currentProfile.commands.empty()) {
      powerCmd = currentProfile.commands[0];
    }
  }
} // namespace

namespace MatchMode {

  void enter(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    mState = MState::CATEGORY_SELECT;
    categoryIndex = 0;
    remoteIndex = 0;
    if (DeviceDatabase::getCategoryCount() > 0) {
      drawCategorySelect(ui, storage);
    } else {
      ui.drawSignalInfo("Error", "No database", "", "", "Run build_db.py");
    }
  }

  void update(UIScreen& ui, SignalStorage& storage, IRController& ir) {
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (mState == MState::CATEGORY_SELECT) {
      size_t count = DeviceDatabase::getCategoryCount();
      if (count == 0) return;
      categoryIndex = (categoryIndex + 1) % count;
      drawCategorySelect(ui, storage);
    } else if (mState == MState::REMOTE_SELECT) {
      size_t count = DeviceDatabase::getProfileCount(categoryIndex);
      if (count == 0) return;
      remoteIndex = (remoteIndex + 1) % count;
      drawRemoteSelect(ui, storage);
    } else if (mState == MState::TEST_POWER) {
      // Short press = re-send
      Signal testSignal;
      testSignal.protocol = currentProfile.protocol;
      testSignal.address = currentProfile.address;
      testSignal.command = powerCmd.hexCode;
      bool success = ir.sendSignal(testSignal);
      drawTestPower(ui, storage, success);
    } else if (mState == MState::SAVE_CONFIRM) {
      mState = MState::TEST_POWER;
      drawTestPower(ui, storage);
    }
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (mState == MState::CATEGORY_SELECT) {
      remoteIndex = 0;
      mState = MState::REMOTE_SELECT;
      drawRemoteSelect(ui, storage);
      return false;
    } else if (mState == MState::REMOTE_SELECT) {
      selectCurrentProfile();
      if (!currentStaticProfile) return false;
      
      mState = MState::TEST_POWER;
      // Auto-transmit power
      Signal testSignal;
      testSignal.protocol = currentProfile.protocol;
      testSignal.address = currentProfile.address;
      testSignal.command = powerCmd.hexCode;
      bool success = ir.sendSignal(testSignal);
      drawTestPower(ui, storage, success);
      return false;
    } else if (mState == MState::TEST_POWER) {
      mState = MState::SAVE_CONFIRM;
      drawSaveConfirm(ui, storage);
      return false;
    } else if (mState == MState::SAVE_CONFIRM) {
      if (storage.getCount() < SignalStorage::MAX_PROFILES) {
        storage.addProfile(currentProfile);
      }
      mState = MState::CATEGORY_SELECT;
      drawCategorySelect(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (mState == MState::CATEGORY_SELECT) {
      return true; // back to main menu
    } else if (mState == MState::REMOTE_SELECT) {
      mState = MState::CATEGORY_SELECT;
      drawCategorySelect(ui, storage);
      return false;
    } else if (mState == MState::TEST_POWER) {
      mState = MState::REMOTE_SELECT;
      drawRemoteSelect(ui, storage);
      return false;
    } else if (mState == MState::SAVE_CONFIRM) {
      mState = MState::TEST_POWER;
      drawTestPower(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    return true;
  }

} // namespace MatchMode
