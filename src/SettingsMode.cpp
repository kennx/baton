#include "SettingsMode.h"

namespace {
  enum class SetSubState { MENU, CONFIRM_CLEAR, ABOUT };
  SetSubState subState = SetSubState::MENU;

  int menuIndex = 0;
  int repeatCount = 2;
  int brightness = 80;
  bool buzzerEnabled = true;
  int confirmOption = 0;

  const std::vector<std::string> MENU_ITEMS = {
    "Clear all",
    "Repeat",
    "Brightness",
    "Buzzer",
    "About"
  };

  void drawMenu(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> items = MENU_ITEMS;
    items[1] += " (" + std::to_string(repeatCount) + ")";
    items[2] += " (" + std::to_string(brightness) + "%)";
    items[3] += buzzerEnabled ? " (ON)" : " (OFF)";

    ui.drawStatusBar("[B]", storage.getCount(), "[S]SET");
    ui.drawMenu(items, menuIndex);
    ui.drawFooter("Short:NXT Long:ADJ");
  }

  void drawConfirmClear(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> opts = {"Cancel", "Confirm"};
    ui.drawPopup("[!] Clear?", "Clear all?\nCannot undo!", opts, confirmOption);
  }

  void drawAbout(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "[S]About");
    ui.drawSignalInfo("IR Scanner", "v1.0.0", "Sig:" + std::to_string(storage.getCount()),
                      "Mem:" + std::to_string(storage.getCount()) + "/50", "Long:Back");
    ui.drawFooter("Long:Back");
  }
}

namespace SettingsMode {

  void enter(UIScreen& ui, SignalStorage& storage) {
    subState = SetSubState::MENU;
    menuIndex = 0;
    drawMenu(ui, storage);
  }

  void update(UIScreen& ui, SignalStorage& storage) {
    // 无Auto状态转换
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == SetSubState::MENU) {
      menuIndex = (menuIndex + 1) % MENU_ITEMS.size();
      drawMenu(ui, storage);
    } else if (subState == SetSubState::CONFIRM_CLEAR) {
      confirmOption = (confirmOption + 1) % 2;
      drawConfirmClear(ui, storage);
    } else if (subState == SetSubState::ABOUT) {
      // ABOUT 状态无操作
    }
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == SetSubState::MENU) {
      if (menuIndex == 0) {  // Clear all
        subState = SetSubState::CONFIRM_CLEAR;
        confirmOption = 0;
        drawConfirmClear(ui, storage);
      } else if (menuIndex == 1) {  // Repeat
        repeatCount++;
        if (repeatCount > 5) repeatCount = 1;
        drawMenu(ui, storage);
      } else if (menuIndex == 2) {  // Brightness
        brightness += 20;
        if (brightness > 100) brightness = 20;
        ui.setBrightness(static_cast<uint8_t>(brightness));
        drawMenu(ui, storage);
      } else if (menuIndex == 3) {  // Buzzer
        buzzerEnabled = !buzzerEnabled;
        drawMenu(ui, storage);
      } else if (menuIndex == 4) {  // About
        subState = SetSubState::ABOUT;
        drawAbout(ui, storage);
      }
      return false;
    } else if (subState == SetSubState::CONFIRM_CLEAR) {
      if (confirmOption == 1) {  // Confirm
        storage.clearAll();
      }
      subState = SetSubState::MENU;
      drawMenu(ui, storage);
      return false;
    } else if (subState == SetSubState::ABOUT) {
      subState = SetSubState::MENU;
      drawMenu(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == SetSubState::MENU) {
      return true;  // back to main menu
    } else if (subState == SetSubState::CONFIRM_CLEAR) {
      subState = SetSubState::MENU;
      drawMenu(ui, storage);
      return false;  // stayed inside Settings
    } else if (subState == SetSubState::ABOUT) {
      subState = SetSubState::MENU;
      drawMenu(ui, storage);
      return false;
    }
    return true;
  }
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage) {
    return true;  // always return to main menu
  }

  int getRepeatCount() { return repeatCount; }
  int getBrightness() { return brightness; }
  bool getBuzzerEnabled() { return buzzerEnabled; }

}
