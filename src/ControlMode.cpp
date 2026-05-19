#include "ControlMode.h"
#include "SettingsMode.h"

namespace {
  enum class CtrlSubState { LIST, CATEGORY_SELECT, SENDING };
  CtrlSubState subState = CtrlSubState::LIST;

  std::vector<Signal> currentSignals;
  std::string currentCategory = "全部";
  int selectedIndex = 0;
  unsigned long sendStartTime = 0;
  static constexpr unsigned long SEND_DISPLAY_MS = 800;

  const std::vector<std::string> CATEGORIES = {"全部", "电视", "空调", "风扇", "机顶盒", "未分类"};
  int categoryIndex = 0;

  void refreshList(SignalStorage& storage) {
    if (currentCategory == "全部") {
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
      items.push_back("(无信号)");
    }
    items.push_back("── 筛选 ──");

    std::string title = currentCategory + "(" + std::to_string(currentSignals.size()) + ")";
    ui.drawStatusBar("🔋", storage.getCount(), "📡控制");
    ui.drawMenu(items, selectedIndex, title);
    ui.drawFooter("短按:下选 长按:发射");
  }

  void drawCategorySelect(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("🔋", storage.getCount(), "📡筛选");
    ui.drawMenu(CATEGORIES, categoryIndex, "选择分类");
    ui.drawFooter("短按:下选 长按:确认");
  }

  void drawSending(UIScreen& ui, const Signal& s) {
    ui.drawStatusBar("🔋", 0, "📡发送中");
    ui.drawSignalInfo(s.name, s.protocol, s.address, s.command, "📡 发送中...");
    ui.drawFooter("请稍候...");
  }
}

namespace ControlMode {

  void enter(UIScreen& ui, SignalStorage& storage) {
    subState = CtrlSubState::LIST;
    currentCategory = "全部";
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
      int itemCount = static_cast<int>(currentSignals.size()) + 1;  // +1 为筛选项
      if (currentSignals.empty()) itemCount = 2;  // (无信号) + 筛选

      selectedIndex++;
      if (selectedIndex >= itemCount) {
        selectedIndex = 0;
      }

      // 最后一项是筛选入口
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
        return true;  // 回到主菜单
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
      return false;  // 不返回主菜单，显示发送结果
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

}
