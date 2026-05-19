#include "ControlMode.h"
#include "SettingsMode.h"

namespace {
  enum class CtrlSubState { LIST, CATEGORY_SELECT, SENDING };
  CtrlSubState subState = CtrlSubState::LIST;

  std::vector<Signal> currentSignals;
  std::string currentCategory = "All";
  int selectedIndex = 0;
  unsigned long sendStartTime = 0;
  static constexpr unsigned long SEND_DISPLAY_MS = 800;

  const std::vector<std::string> CATEGORIES = {"All", "TV", "AC", "FAN", "STB", "Uncat"};
  int categoryIndex = 0;

  void refreshList(SignalStorage& storage) {
    if (currentCategory == "All") {
      currentSignals = storage.getAllSignals();
    } else {
      currentSignals = storage.getSignalsByCategory(currentCategory);
    }
    if (selectedIndex >= static_cast<int>(currentSignals.size())) {
      selectedIndex = 0;
    }
  }

  void drawList(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> items;
    for (const auto& s : currentSignals) {
      items.push_back(s.name);
    }
    if (items.empty()) {
      items.push_back("(no signal)");
    }
    items.push_back("-- FILTER --");

    std::string title = currentCategory + "(" + std::to_string(currentSignals.size()) + ")";
    ui.drawStatusBar("[B]", storage.getCount(), "[T]CTRL");
    ui.drawMenu(items, selectedIndex, title);
    ui.drawFooter("Short:NXT Long:SEND");
  }

  void drawCategorySelect(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "[T]FILTER");
    ui.drawMenu(CATEGORIES, categoryIndex, "Pick category");
    ui.drawFooter("Short:NXT Long:OK");
  }

  void drawSending(UIScreen& ui, const Signal& s) {
    ui.drawStatusBar("[B]", 0, "[T]SENDING");
    ui.drawSignalInfo(s.name, s.protocol, s.address, s.command, "[T] SENDING...");
    ui.drawFooter("Wait...");
  }
}

namespace ControlMode {

  void enter(UIScreen& ui, SignalStorage& storage) {
    subState = CtrlSubState::LIST;
    currentCategory = "All";
    selectedIndex = 0;
    refreshList(storage);
    drawList(ui, storage);
  }

  void update(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (subState == CtrlSubState::SENDING) {
      if (millis() - sendStartTime >= SEND_DISPLAY_MS) {
        subState = CtrlSubState::LIST;
        drawList(ui, storage);
      }
    }
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == CtrlSubState::LIST) {
      int itemCount = static_cast<int>(currentSignals.size()) + 1;  // +1 为FILTER项
      if (currentSignals.empty()) itemCount = 2;  // (no signal) + FILTER

      selectedIndex++;
      if (selectedIndex >= itemCount) {
        selectedIndex = 0;
      }

      // 最后一项是FILTER入口
      if (!currentSignals.empty() && selectedIndex == static_cast<int>(currentSignals.size())) {
        subState = CtrlSubState::CATEGORY_SELECT;
        categoryIndex = 0;
        drawCategorySelect(ui, storage);
      } else if (currentSignals.empty() && selectedIndex == 1) {
        subState = CtrlSubState::CATEGORY_SELECT;
        categoryIndex = 0;
        drawCategorySelect(ui, storage);
      } else {
        drawList(ui, storage);
      }
    } else if (subState == CtrlSubState::CATEGORY_SELECT) {
      categoryIndex = (categoryIndex + 1) % CATEGORIES.size();
      drawCategorySelect(ui, storage);
    }
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (subState == CtrlSubState::LIST) {
      if (currentSignals.empty()) {
        return true;  // 回到MENU
      }
      if (selectedIndex >= 0 && selectedIndex < static_cast<int>(currentSignals.size())) {
        const Signal& s = currentSignals[selectedIndex];

        // 读取完整信号（含 rawData）
        Signal fullSignal = storage.getSignal(s.id);

        // 发射
        subState = CtrlSubState::SENDING;
        sendStartTime = millis();
        drawSending(ui, fullSignal);

        // 发射（含重复）
        int repeatCount = SettingsMode::getRepeatCount();
        for (int i = 0; i < repeatCount; i++) {
          ir.sendSignal(fullSignal);
          if (i < repeatCount - 1) delay(100);
        }
      }
      return false;  // 不BackMENU，显示发送结果
    } else if (subState == CtrlSubState::CATEGORY_SELECT) {
      currentCategory = CATEGORIES[categoryIndex];
      selectedIndex = 0;
      subState = CtrlSubState::LIST;
      refreshList(storage);
      drawList(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == CtrlSubState::LIST) {
      return true;  // back to main menu
    } else if (subState == CtrlSubState::CATEGORY_SELECT) {
      subState = CtrlSubState::LIST;
      selectedIndex = 0;
      drawList(ui, storage);
      return false;
    } else if (subState == CtrlSubState::SENDING) {
      subState = CtrlSubState::LIST;
      drawList(ui, storage);
      return false;
    }
    return true;
  }
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage) {
    return true;  // always back to main menu
  }

}
