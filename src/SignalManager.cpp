#include "SignalManager.h"

namespace {
  enum class MgrSubState { SIGNAL_LIST, ACTION_MENU, RENAME_CATEGORY, RENAME_NAME, CATEGORY_SELECT, DELETE_CONFIRM, RAW_DATA };
  MgrSubState subState = MgrSubState::SIGNAL_LIST;

  std::vector<Signal> signals;
  int signalIndex = 0;
  int actionIndex = 0;
  int renameCatIndex = 0;
  int renameNameIndex = 0;
  int deleteOption = 0;

  const std::vector<std::string> ACTIONS = {"重命名", "归类到文件夹", "删除信号", "查看原始数据", "返回"};
  const std::vector<std::string> RENAME_CATS = {"电视", "空调", "风扇", "机顶盒", "自定义"};

  const std::vector<std::string> TV_NAMES = {"电源", "音量+", "音量-", "频道+", "频道-", "静音", "菜单", "上", "下", "左", "右", "确认", "返回"};
  const std::vector<std::string> AC_NAMES = {"电源", "制冷", "制热", "除湿", "自动", "升温", "降温", "风速", "扫风", "定时"};
  const std::vector<std::string> FAN_NAMES = {"电源", "风速", "摇头", "定时"};
  const std::vector<std::string> STB_NAMES = {"电源", "菜单", "回看", "直播", "上", "下", "左", "右", "确认", "返回"};
  const std::vector<std::string> CUSTOM_NAMES = {"自定义-01", "自定义-02", "自定义-03", "自定义-04", "自定义-05"};

  const std::vector<std::string>* currentNameList = &TV_NAMES;

  void refreshSignals(SignalStorage& storage) {
    signals = storage.getAllSignals();
    if (signalIndex >= static_cast<int>(signals.size())) {
      signalIndex = 0;
    }
  }

  void drawSignalList(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> items;
    for (const auto& s : signals) {
      items.push_back(s.name + " [" + s.category + "]");
    }
    if (items.empty()) {
      items.push_back("(无信号)");
    }
    ui.drawStatusBar("🔋", storage.getCount(), "📁管理");
    ui.drawMenu(items, signalIndex, "选择信号");
    ui.drawFooter("短按:下选 长按:管理");
  }

  void drawActionMenu(UIScreen& ui, SignalStorage& storage) {
    std::string title = signals[signalIndex].name;
    ui.drawStatusBar("🔋", storage.getCount(), "📁管理");
    ui.drawMenu(ACTIONS, actionIndex, title);
    ui.drawFooter("短按:下选 长按:确认");
  }

  void drawRenameCategory(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("🔋", storage.getCount(), "📁重命名");
    ui.drawMenu(RENAME_CATS, renameCatIndex, "选择分类");
    ui.drawFooter("短按:下选 长按:确认");
  }

  void drawRenameName(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("🔋", storage.getCount(), "📁重命名");
    ui.drawMenu(*currentNameList, renameNameIndex, "选择名称");
    ui.drawFooter("短按:下选 长按:确认");
  }

  void drawCategorySelect(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> cats = {"电视", "空调", "风扇", "机顶盒", "未分类"};
    ui.drawStatusBar("🔋", storage.getCount(), "📁归类");
    ui.drawMenu(cats, renameCatIndex, "选择分类");
    ui.drawFooter("短按:下选 长按:确认");
  }

  void drawDeleteConfirm(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> opts = {"取消", "确认删除"};
    ui.drawPopup("⚠️ 删除确认", "确认删除 " + signals[signalIndex].name + "?\n此操作不可恢复", opts, deleteOption);
  }

  void drawRawData(UIScreen& ui, SignalStorage& storage) {
    Signal s = storage.getSignal(signals[signalIndex].id);
    ui.drawStatusBar("🔋", storage.getCount(), "📁原始数据");
    ui.drawSignalInfo(s.name, s.protocol, s.address, s.command, "Raw: " + std::to_string(s.rawLength));
    ui.drawFooter("长按:返回");
  }

  const std::vector<std::string>* getNameList(const std::string& category) {
    if (category == "电视") return &TV_NAMES;
    if (category == "空调") return &AC_NAMES;
    if (category == "风扇") return &FAN_NAMES;
    if (category == "机顶盒") return &STB_NAMES;
    return &CUSTOM_NAMES;
  }
}

namespace SignalManager {

  void enter(UIScreen& ui, SignalStorage& storage) {
    subState = MgrSubState::SIGNAL_LIST;
    signalIndex = 0;
    refreshSignals(storage);
    drawSignalList(ui, storage);
  }

  void update(UIScreen& ui, SignalStorage& storage) {
    // 无自动状态转换，全靠按钮驱动
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == MgrSubState::SIGNAL_LIST) {
      if (!signals.empty()) {
        signalIndex = (signalIndex + 1) % signals.size();
      }
      drawSignalList(ui, storage);
    } else if (subState == MgrSubState::ACTION_MENU) {
      actionIndex = (actionIndex + 1) % ACTIONS.size();
      drawActionMenu(ui, storage);
    } else if (subState == MgrSubState::RENAME_CATEGORY) {
      renameCatIndex = (renameCatIndex + 1) % RENAME_CATS.size();
      drawRenameCategory(ui, storage);
    } else if (subState == MgrSubState::RENAME_NAME) {
      renameNameIndex = (renameNameIndex + 1) % currentNameList->size();
      drawRenameName(ui, storage);
    } else if (subState == MgrSubState::CATEGORY_SELECT) {
      std::vector<std::string> cats = {"电视", "空调", "风扇", "机顶盒", "未分类"};
      renameCatIndex = (renameCatIndex + 1) % cats.size();
      drawCategorySelect(ui, storage);
    } else if (subState == MgrSubState::DELETE_CONFIRM) {
      deleteOption = (deleteOption + 1) % 2;
      drawDeleteConfirm(ui, storage);
    }
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == MgrSubState::SIGNAL_LIST) {
      if (signals.empty()) return true;
      subState = MgrSubState::ACTION_MENU;
      actionIndex = 0;
      drawActionMenu(ui, storage);
      return false;
    } else if (subState == MgrSubState::ACTION_MENU) {
      if (actionIndex == 0) {  // 重命名
        subState = MgrSubState::RENAME_CATEGORY;
        renameCatIndex = 0;
        drawRenameCategory(ui, storage);
      } else if (actionIndex == 1) {  // 归类
        subState = MgrSubState::CATEGORY_SELECT;
        renameCatIndex = 0;
        drawCategorySelect(ui, storage);
      } else if (actionIndex == 2) {  // 删除
        subState = MgrSubState::DELETE_CONFIRM;
        deleteOption = 0;
        drawDeleteConfirm(ui, storage);
      } else if (actionIndex == 3) {  // 原始数据
        subState = MgrSubState::RAW_DATA;
        drawRawData(ui, storage);
      } else if (actionIndex == 4) {  // 返回
        subState = MgrSubState::SIGNAL_LIST;
        drawSignalList(ui, storage);
      }
      return false;
    } else if (subState == MgrSubState::RENAME_CATEGORY) {
      std::string cat = RENAME_CATS[renameCatIndex];
      currentNameList = getNameList(cat);
      subState = MgrSubState::RENAME_NAME;
      renameNameIndex = 0;
      drawRenameName(ui, storage);
      return false;
    } else if (subState == MgrSubState::RENAME_NAME) {
      std::string cat = RENAME_CATS[renameCatIndex];
      std::string name = (*currentNameList)[renameNameIndex];
      Signal s = signals[signalIndex];
      s.name = cat + "-" + name;
      s.category = cat;
      // 更新存储：删除旧信号，添加新信号（保持其他字段）
      storage.removeSignal(signals[signalIndex].id);
      s.id = -1;  // 让 addSignal 重新分配 id
      storage.addSignal(s);
      refreshSignals(storage);
      subState = MgrSubState::SIGNAL_LIST;
      drawSignalList(ui, storage);
      return false;
    } else if (subState == MgrSubState::CATEGORY_SELECT) {
      std::vector<std::string> cats = {"电视", "空调", "风扇", "机顶盒", "未分类"};
      std::string cat = cats[renameCatIndex];
      Signal s = signals[signalIndex];
      s.category = cat;
      storage.removeSignal(signals[signalIndex].id);
      s.id = -1;
      storage.addSignal(s);
      refreshSignals(storage);
      subState = MgrSubState::SIGNAL_LIST;
      drawSignalList(ui, storage);
      return false;
    } else if (subState == MgrSubState::DELETE_CONFIRM) {
      if (deleteOption == 1) {  // 确认删除
        storage.removeSignal(signals[signalIndex].id);
        refreshSignals(storage);
      }
      subState = MgrSubState::SIGNAL_LIST;
      drawSignalList(ui, storage);
      return false;
    } else if (subState == MgrSubState::RAW_DATA) {
      subState = MgrSubState::SIGNAL_LIST;
      drawSignalList(ui, storage);
      return false;
    }
    return true;
  }

}
