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
    "清空所有信号",
    "发射重复次数",
    "屏幕亮度",
    "蜂鸣器开关",
    "关于"
  };

  void drawMenu(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> items = MENU_ITEMS;
    items[1] += " (" + std::to_string(repeatCount) + ")";
    items[2] += " (" + std::to_string(brightness) + "%)";
    items[3] += buzzerEnabled ? " (开)" : " (关)";

    ui.drawStatusBar("🔋", storage.getCount(), "⚙️设置");
    ui.drawMenu(items, menuIndex);
    ui.drawFooter("短按:下选 长按:调整");
  }

  void drawConfirmClear(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> opts = {"取消", "确认清空"};
    ui.drawPopup("⚠️ 清空确认", "确认删除所有信号?\n此操作不可恢复", opts, confirmOption);
  }

  void drawAbout(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("🔋", storage.getCount(), "⚙️关于");
    ui.drawSignalInfo("IR Scanner", "v1.0.0", "信号: " + std::to_string(storage.getCount()),
                      "存储: " + std::to_string(storage.getCount()) + "/50", "长按返回");
    ui.drawFooter("长按:返回");
  }
}

namespace SettingsMode {

  void enter(UIScreen& ui, SignalStorage& storage) {
    subState = SetSubState::MENU;
    menuIndex = 0;
    drawMenu(ui, storage);
  }

  void update(UIScreen& ui, SignalStorage& storage) {
    // 无自动状态转换
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
      if (menuIndex == 0) {  // 清空所有信号
        subState = SetSubState::CONFIRM_CLEAR;
        confirmOption = 0;
        drawConfirmClear(ui, storage);
      } else if (menuIndex == 1) {  // 发射重复次数
        repeatCount++;
        if (repeatCount > 5) repeatCount = 1;
        drawMenu(ui, storage);
      } else if (menuIndex == 2) {  // 屏幕亮度
        brightness += 20;
        if (brightness > 100) brightness = 20;
        ui.setBrightness(static_cast<uint8_t>(brightness));
        drawMenu(ui, storage);
      } else if (menuIndex == 3) {  // 蜂鸣器开关
        buzzerEnabled = !buzzerEnabled;
        drawMenu(ui, storage);
      } else if (menuIndex == 4) {  // 关于
        subState = SetSubState::ABOUT;
        drawAbout(ui, storage);
      }
      return false;
    } else if (subState == SetSubState::CONFIRM_CLEAR) {
      if (confirmOption == 1) {  // 确认清空
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

  int getRepeatCount() { return repeatCount; }
  int getBrightness() { return brightness; }
  bool getBuzzerEnabled() { return buzzerEnabled; }

}
