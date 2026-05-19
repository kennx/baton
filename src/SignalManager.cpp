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

  const std::vector<std::string> ACTIONS = {"Rename", "Categorize", "Delete", "Raw data", "Back"};
  const std::vector<std::string> RENAME_CATS = {"TV", "AC", "FAN", "STB", "Custom"};

  const std::vector<std::string> TV_NAMES = {"Power", "Vol+", "Vol-", "CH+", "CH-", "Mute", "Menu", "Up", "Down", "Left", "Right", "OK", "Back"};
  const std::vector<std::string> AC_NAMES = {"Power", "Cool", "Heat", "Dry", "Auto", "Temp+", "Temp-", "Fan", "Swing", "Timer"};
  const std::vector<std::string> FAN_NAMES = {"Power", "Fan", "Osc", "Timer"};
  const std::vector<std::string> STB_NAMES = {"Power", "Menu", "Back", "Live", "Up", "Down", "Left", "Right", "OK", "Back"};
  const std::vector<std::string> CUSTOM_NAMES = {"Custom-01", "Custom-02", "Custom-03", "Custom-04", "Custom-05"};

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
      items.push_back("(no signal)");
    }
    ui.drawStatusBar("[B]", storage.getCount(), "[M]MGMT");
    ui.drawMenu(items, signalIndex, "Pick signal");
    ui.drawFooter("Short:NXT Long:MGMT");
  }

  void drawActionMenu(UIScreen& ui, SignalStorage& storage) {
    std::string title = signals[signalIndex].name;
    ui.drawStatusBar("[B]", storage.getCount(), "[M]MGMT");
    ui.drawMenu(ACTIONS, actionIndex, title);
    ui.drawFooter("Short:NXT Long:OK");
  }

  void drawRenameCategory(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "[M]Rename");
    ui.drawMenu(RENAME_CATS, renameCatIndex, "Pick category");
    ui.drawFooter("Short:NXT Long:OK");
  }

  void drawRenameName(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "[M]Rename");
    ui.drawMenu(*currentNameList, renameNameIndex, "Pick name");
    ui.drawFooter("Short:NXT Long:OK");
  }

  void drawCategorySelect(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> cats = {"TV", "AC", "FAN", "STB", "Uncat"};
    ui.drawStatusBar("[B]", storage.getCount(), "[M]CAT");
    ui.drawMenu(cats, renameCatIndex, "Pick category");
    ui.drawFooter("Short:NXT Long:OK");
  }

  void drawDeleteConfirm(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> opts = {"Cancel", "Confirm"};
    ui.drawPopup("[!] DEL?", "Del " + signals[signalIndex].name + "?\nCannot undo!", opts, deleteOption);
  }

  void drawRawData(UIScreen& ui, SignalStorage& storage) {
    Signal s = storage.getSignal(signals[signalIndex].id);
    ui.drawStatusBar("[B]", storage.getCount(), "[M]RAW");
    ui.drawSignalInfo(s.name, s.protocol, s.address, s.command, "Raw: " + std::to_string(s.rawLength));
    ui.drawFooter("Long:Back");
  }

  const std::vector<std::string>* getNameList(const std::string& category) {
    if (category == "TV") return &TV_NAMES;
    if (category == "AC") return &AC_NAMES;
    if (category == "FAN") return &FAN_NAMES;
    if (category == "STB") return &STB_NAMES;
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
    // 无Auto状态转换，全靠按钮驱动
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
      std::vector<std::string> cats = {"TV", "AC", "FAN", "STB", "Uncat"};
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
      if (actionIndex == 0) {  // Rename
        subState = MgrSubState::RENAME_CATEGORY;
        renameCatIndex = 0;
        drawRenameCategory(ui, storage);
      } else if (actionIndex == 1) {  // CAT
        subState = MgrSubState::CATEGORY_SELECT;
        renameCatIndex = 0;
        drawCategorySelect(ui, storage);
      } else if (actionIndex == 2) {  // 删除
        subState = MgrSubState::DELETE_CONFIRM;
        deleteOption = 0;
        drawDeleteConfirm(ui, storage);
      } else if (actionIndex == 3) {  // RAW
        subState = MgrSubState::RAW_DATA;
        drawRawData(ui, storage);
      } else if (actionIndex == 4) {  // Back
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
      std::vector<std::string> cats = {"TV", "AC", "FAN", "STB", "Uncat"};
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
      if (deleteOption == 1) {  // Confirm
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

  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage) {
    if (subState == MgrSubState::SIGNAL_LIST) {
      return true;  // back to main menu
    } else if (subState == MgrSubState::ACTION_MENU) {
      subState = MgrSubState::SIGNAL_LIST;
      drawSignalList(ui, storage);
      return false;
    } else if (subState == MgrSubState::RENAME_CATEGORY) {
      subState = MgrSubState::ACTION_MENU;
      drawActionMenu(ui, storage);
      return false;
    } else if (subState == MgrSubState::RENAME_NAME) {
      subState = MgrSubState::RENAME_CATEGORY;
      drawRenameCategory(ui, storage);
      return false;
    } else if (subState == MgrSubState::CATEGORY_SELECT) {
      subState = MgrSubState::ACTION_MENU;
      drawActionMenu(ui, storage);
      return false;
    } else if (subState == MgrSubState::DELETE_CONFIRM) {
      subState = MgrSubState::ACTION_MENU;
      drawActionMenu(ui, storage);
      return false;
    } else if (subState == MgrSubState::RAW_DATA) {
      subState = MgrSubState::SIGNAL_LIST;
      drawSignalList(ui, storage);
      return false;
    }
    return true;
  }
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage) {
    return true;  // always back to main menu
  }

}
