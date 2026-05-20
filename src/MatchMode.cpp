#include "MatchMode.h"
#include <M5Unified.h>

namespace {
  enum class MState { BRAND_SELECT, TEST_POWER, SAVE_CONFIRM };
  MState mState = MState::BRAND_SELECT;

  std::vector<DeviceProfile> allProfiles;
  int brandIndex = 0;
  
  DeviceProfile currentProfile;
  DeviceCommand powerCmd;

  void drawBrandSelect(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> brands;
    for (const auto& p : allProfiles) {
      brands.push_back(p.brand);
    }
    ui.drawStatusBar("[B]", storage.getCount(), "MATCH");
    ui.drawMenu(brands, brandIndex, "Select Brand");
    ui.drawFooter("NXT           TEST");
  }

  void drawTestPower(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "TEST");
    ui.drawSignalInfo(currentProfile.brand, currentProfile.protocol, currentProfile.address, powerCmd.hexCode, "Test Power");
    ui.drawFooter("TEST          SAVE");
  }

  void drawSaveConfirm(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "SAVE");
    ui.drawSignalInfo("Save Device?", currentProfile.brand, "", "", "Hold to confirm");
    ui.drawFooter("BACK          SAVE");
  }
} // namespace

namespace MatchMode {

  void enter(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    mState = MState::BRAND_SELECT;
    allProfiles = DeviceDatabase::getAllProfiles();
    brandIndex = 0;
    if (!allProfiles.empty()) {
      drawBrandSelect(ui, storage);
    } else {
      ui.drawSignalInfo("Error", "No profiles", "", "", "");
    }
  }

  void update(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    // No continuous logic needed for match mode
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (allProfiles.empty()) return;
    if (mState == MState::BRAND_SELECT) {
      brandIndex = (brandIndex + 1) % allProfiles.size();
      drawBrandSelect(ui, storage);
    } else if (mState == MState::TEST_POWER) {
      // Re-transmit power
      Signal testSignal;
      testSignal.protocol = currentProfile.protocol;
      testSignal.address = currentProfile.address;
      testSignal.command = powerCmd.hexCode;
      ir.sendSignal(testSignal);
      drawTestPower(ui, storage);
    } else if (mState == MState::SAVE_CONFIRM) {
      mState = MState::TEST_POWER;
      drawTestPower(ui, storage);
    }
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (allProfiles.empty()) return true;

    if (mState == MState::BRAND_SELECT) {
      currentProfile = allProfiles[brandIndex];
      // Find power command
      powerCmd = {"Power", "0x00"};
      for (const auto& c : currentProfile.commands) {
        if (c.name == "Power") {
          powerCmd = c;
          break;
        }
      }
      
      mState = MState::TEST_POWER;
      
      // Auto-transmit power on enter
      Signal testSignal;
      testSignal.protocol = currentProfile.protocol;
      testSignal.address = currentProfile.address;
      testSignal.command = powerCmd.hexCode;
      ir.sendSignal(testSignal);
      
      drawTestPower(ui, storage);
      return false;
    } else if (mState == MState::TEST_POWER) {
      mState = MState::SAVE_CONFIRM;
      drawSaveConfirm(ui, storage);
      return false;
    } else if (mState == MState::SAVE_CONFIRM) {
      if (storage.getCount() < SignalStorage::MAX_PROFILES) {
        storage.addProfile(currentProfile);
      }
      mState = MState::BRAND_SELECT;
      drawBrandSelect(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (mState == MState::BRAND_SELECT) {
      return true; // go back to main menu
    } else if (mState == MState::TEST_POWER) {
      mState = MState::BRAND_SELECT;
      drawBrandSelect(ui, storage);
      return false;
    } else if (mState == MState::SAVE_CONFIRM) {
      mState = MState::TEST_POWER;
      drawTestPower(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    return true; // return to menu
  }

} // namespace MatchMode
