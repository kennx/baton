# BruteForce + btnB Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add BruteForce mode (protocol enumeration to find IR codes without a remote) and btnB back button (short=back one level, long=back to main menu) to the existing IR scanner.

**Architecture:** Minimal-invasive additions: (1) add btnB detection alongside btnA in AppStateMachine, add `onBtnB*` callbacks to all existing modes; (2) create new `BruteForceMode` namespace with its own state machine; (3) extract preset name lists to a shared header for reuse between SignalManager and BruteForceMode.

**Tech Stack:** PlatformIO, ESP32-S3 Arduino, M5Unified, IRremoteESP8266, LittleFS. No new libraries.

---

## File Structure

| File | Action | Responsibility |
|------|--------|---------------|
| `src/AppStateMachine.h` | Modify | Add `AppState::BRUTE_FORCE`, btnB state tracking, rename `checkButton`/`isShortPress`/`isLongPress` |
| `src/AppStateMachine.cpp` | Modify | Split into `checkBtnA`/`checkBtnB`, add `onBtnBShortPress`/`onBtnBLongPress`, integrate BruteForceMode |
| `src/ScanMode.h` | Modify | Add `onBtnBShortPress`/`onBtnBLongPress` declarations |
| `src/ScanMode.cpp` | Modify | Implement btnB callbacks |
| `src/ControlMode.h` | Modify | Add `onBtnBShortPress`/`onBtnBLongPress` declarations |
| `src/ControlMode.cpp` | Modify | Implement btnB callbacks (sub-state backtracking) |
| `src/SignalManager.h` | Modify | Add `onBtnBShortPress`/`onBtnBLongPress` declarations |
| `src/SignalManager.cpp` | Modify | Implement btnB callbacks (deep sub-state backtracking), switch to `PresetNames` |
| `src/SettingsMode.h` | Modify | Add `onBtnBShortPress`/`onBtnBLongPress` declarations |
| `src/SettingsMode.cpp` | Modify | Implement btnB callbacks |
| `src/PresetNames.h` | **Create** | Shared preset name/category constants |
| `src/BruteForceMode.h` | **Create** | Header with `enter`/`update`/`onShortPress`/`onLongPress`/`onBtnBShortPress`/`onBtnBLongPress` |
| `src/BruteForceMode.cpp` | **Create** | Full implementation: protocol selection, auto-step enumeration, hit save |

---

## Task 1: btnB Infrastructure — All Modes + AppStateMachine

**Files:**
- Modify: `src/ScanMode.h`, `src/ScanMode.cpp`
- Modify: `src/ControlMode.h`, `src/ControlMode.cpp`
- Modify: `src/SignalManager.h`, `src/SignalManager.cpp`
- Modify: `src/SettingsMode.h`, `src/SettingsMode.cpp`
- Modify: `src/AppStateMachine.h`, `src/AppStateMachine.cpp`

### Step 1: Add `onBtnB*` declarations to all 4 mode headers

In `src/ScanMode.h`, add after `onLongPress`:
```cpp
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
```

In `src/ControlMode.h`, add after `onLongPress`:
```cpp
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage);
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage);
```

In `src/SignalManager.h`, add after `onLongPress`:
```cpp
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage);
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage);
```

In `src/SettingsMode.h`, add after `onLongPress`:
```cpp
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage);
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage);
```

### Step 2: Add stub implementations to all 4 mode cpp files

In `src/ScanMode.cpp`, add at end of namespace:
```cpp
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    ir.disableReceive();
    return true;
  }
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    ir.disableReceive();
    return true;
  }
```

In `src/ControlMode.cpp`, add at end of namespace:
```cpp
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage) {
    return true;  // stub: will be filled in Task 3
  }
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage) {
    return true;  // stub
  }
```

In `src/SignalManager.cpp`, add at end of namespace:
```cpp
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage) {
    return true;  // stub: will be filled in Task 3
  }
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage) {
    return true;  // stub
  }
```

In `src/SettingsMode.cpp`, add at end of namespace:
```cpp
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage) {
    return true;  // stub: will be filled in Task 2
  }
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage) {
    return true;  // stub
  }
```

### Step 3: Refactor AppStateMachine for dual-button support

In `src/AppStateMachine.h`:

Rename `checkButton`/`isShortPress`/`isLongPress` and add btnB counterparts:

```cpp
  // Replace: void checkButton(); bool isShortPress(); bool isLongPress();
  void checkBtnA();
  void checkBtnB();
  void onBtnAShortPress();
  void onBtnALongPress();
  void onBtnBShortPress();
  void onBtnBLongPress();
```

Add btnB state members after existing btnA members:
```cpp
  bool btnBWasPressed_ = false;
  unsigned long btnBPressStart_ = 0;
  bool btnBLongHandled_ = false;
```

In `src/AppStateMachine.cpp`:

Replace `checkButton` implementation with `checkBtnA`:
```cpp
void AppStateMachine::checkBtnA() {
  bool pressed = M5.BtnA.isPressed();
  if (pressed && !btnWasPressed_) {
    btnWasPressed_ = true;
    btnPressStart_ = millis();
    longPressHandled_ = false;
  }
  if (!pressed && btnWasPressed_) {
    btnWasPressed_ = false;
    if (!longPressHandled_ && (millis() - btnPressStart_ < LONG_PRESS_MS)) {
      onBtnAShortPress();
    }
  }
  if (btnWasPressed_ && !longPressHandled_ &&
      (millis() - btnPressStart_ >= LONG_PRESS_MS)) {
    longPressHandled_ = true;
    onBtnALongPress();
  }
}
```

Add `checkBtnB` (mirror of checkBtnA):
```cpp
void AppStateMachine::checkBtnB() {
  bool pressed = M5.BtnB.isPressed();
  if (pressed && !btnBWasPressed_) {
    btnBWasPressed_ = true;
    btnBPressStart_ = millis();
    btnBLongHandled_ = false;
  }
  if (!pressed && btnBWasPressed_) {
    btnBWasPressed_ = false;
    if (!btnBLongHandled_ && (millis() - btnBPressStart_ < LONG_PRESS_MS)) {
      onBtnBShortPress();
    }
  }
  if (btnBWasPressed_ && !btnBLongHandled_ &&
      (millis() - btnBPressStart_ >= LONG_PRESS_MS)) {
    btnBLongHandled_ = true;
    onBtnBLongPress();
  }
}
```

In `update()`, replace `checkButton()` with:
```cpp
void AppStateMachine::update() {
  M5.update();
  checkBtnA();
  checkBtnB();
  // ... rest unchanged
}
```

Rename `isShortPress` to `onBtnAShortPress` and `isLongPress` to `onBtnALongPress` (function bodies unchanged except function names).

Add `onBtnBShortPress`:
```cpp
void AppStateMachine::onBtnBShortPress() {
  bool returnToMenu = false;
  switch (state_) {
    case AppState::SCAN_MODE:
      returnToMenu = ScanMode::onBtnBShortPress(ui_, storage_, ir_);
      break;
    case AppState::CONTROL_MODE:
      returnToMenu = ControlMode::onBtnBShortPress(ui_, storage_);
      break;
    case AppState::SIGNAL_MANAGER:
      returnToMenu = SignalManager::onBtnBShortPress(ui_, storage_);
      break;
    case AppState::SETTINGS:
      returnToMenu = SettingsMode::onBtnBShortPress(ui_, storage_);
      break;
  }
  if (returnToMenu) changeState(AppState::MAIN_MENU);
}
```

Add `onBtnBLongPress`:
```cpp
void AppStateMachine::onBtnBLongPress() {
  bool returnToMenu = false;
  switch (state_) {
    case AppState::SCAN_MODE:
      returnToMenu = ScanMode::onBtnBLongPress(ui_, storage_, ir_);
      break;
    case AppState::CONTROL_MODE:
      returnToMenu = ControlMode::onBtnBLongPress(ui_, storage_);
      break;
    case AppState::SIGNAL_MANAGER:
      returnToMenu = SignalManager::onBtnBLongPress(ui_, storage_);
      break;
    case AppState::SETTINGS:
      returnToMenu = SettingsMode::onBtnBLongPress(ui_, storage_);
      break;
  }
  if (returnToMenu) changeState(AppState::MAIN_MENU);
}
```

### Step 4: Compile

Run: `pio run -e m5stick-s3`

Expected: Builds successfully. All btnB stubs return `true`, so btnB will immediately return to main menu in all states (safe fallback).

### Step 5: Commit

```bash
git add src/
git commit -m "feat: btnB infrastructure — add detection, stubs, and AppStateMachine wiring"
```

---

## Task 2: Fill btnB Logic — ScanMode + SettingsMode

**Files:**
- Modify: `src/ScanMode.cpp`
- Modify: `src/SettingsMode.cpp`

### Step 1: Implement ScanMode btnB

Replace stubs in `src/ScanMode.cpp`:
```cpp
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    ir.disableReceive();
    return true;  // return to main menu
  }
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    ir.disableReceive();
    return true;  // return to main menu
  }
```

### Step 2: Implement SettingsMode btnB

Replace stubs in `src/SettingsMode.cpp`:
```cpp
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
```

### Step 3: Compile

Run: `pio run -e m5stick-s3`

Expected: Builds successfully.

### Step 4: Commit

```bash
git add src/ScanMode.cpp src/SettingsMode.cpp
git commit -m "feat: implement btnB back for ScanMode and SettingsMode"
```

---

## Task 3: Fill btnB Logic — ControlMode + SignalManager

**Files:**
- Modify: `src/ControlMode.cpp`
- Modify: `src/SignalManager.cpp`

### Step 1: Implement ControlMode btnB

Replace stubs in `src/ControlMode.cpp`:
```cpp
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
```

### Step 2: Implement SignalManager btnB

Replace stubs in `src/SignalManager.cpp`:
```cpp
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
```

### Step 3: Compile

Run: `pio run -e m5stick-s3`

Expected: Builds successfully.

### Step 4: Commit

```bash
git add src/ControlMode.cpp src/SignalManager.cpp
git commit -m "feat: implement btnB back for ControlMode and SignalManager"
```

---

## Task 4: Extract Shared PresetNames Header

**Files:**
- **Create:** `src/PresetNames.h`
- Modify: `src/SignalManager.cpp`

### Step 1: Create `src/PresetNames.h`

```cpp
#pragma once
#include <vector>
#include <string>

namespace PresetNames {

  const std::vector<std::string> TV = {
    "Power", "Vol+", "Vol-", "CH+", "CH-", "Mute",
    "Menu", "Up", "Down", "Left", "Right", "OK", "Back"
  };
  const std::vector<std::string> AC = {
    "Power", "Cool", "Heat", "Dry", "Auto", "Temp+",
    "Temp-", "Fan", "Swing", "Timer"
  };
  const std::vector<std::string> FAN = {
    "Power", "Fan", "Osc", "Timer"
  };
  const std::vector<std::string> STB = {
    "Power", "Menu", "Back", "Live", "Up", "Down",
    "Left", "Right", "OK", "Back"
  };
  const std::vector<std::string> CUSTOM = {
    "Custom-01", "Custom-02", "Custom-03", "Custom-04", "Custom-05"
  };

  const std::vector<std::string> CATEGORIES = {
    "TV", "AC", "FAN", "STB", "Custom"
  };

  inline const std::vector<std::string>* getNameList(const std::string& category) {
    if (category == "TV") return &TV;
    if (category == "AC") return &AC;
    if (category == "FAN") return &FAN;
    if (category == "STB") return &STB;
    return &CUSTOM;
  }

} // namespace PresetNames
```

### Step 2: Refactor SignalManager to use PresetNames

In `src/SignalManager.cpp`:

Add include:
```cpp
#include "PresetNames.h"
```

Remove the old local vectors:
```cpp
  // REMOVE these lines:
  // const std::vector<std::string> TV_NAMES = {...};
  // const std::vector<std::string> AC_NAMES = {...};
  // const std::vector<std::string> FAN_NAMES = {...};
  // const std::vector<std::string> STB_NAMES = {...};
  // const std::vector<std::string> CUSTOM_NAMES = {...};
  // const std::vector<std::string>* currentNameList = &TV_NAMES;
```

Change `RENAME_CATS` to use `PresetNames::CATEGORIES` (or keep the local `RENAME_CATS` since it only has 5 items — either way works). Simpler: keep local `RENAME_CATS` as-is.

Change `getNameList` to use `PresetNames::getNameList`:
```cpp
  // REMOVE the old getNameList function
```

Update all references from `TV_NAMES` → `PresetNames::TV`, `AC_NAMES` → `PresetNames::AC`, etc.

Update `currentNameList` assignments:
```cpp
  // In drawRenameName and other places:
  // currentNameList = getNameList(cat);
  // becomes:
  // currentNameList = PresetNames::getNameList(cat);
```

Also remove the local `getNameList` function body since `PresetNames::getNameList` replaces it.

### Step 3: Compile

Run: `pio run -e m5stick-s3`

Expected: Builds successfully. Behavior unchanged.

### Step 4: Commit

```bash
git add src/PresetNames.h src/SignalManager.cpp
git commit -m "refactor: extract preset names to shared PresetNames.h for reuse"
```

---

## Task 5: BruteForceMode Implementation

**Files:**
- **Create:** `src/BruteForceMode.h`
- **Create:** `src/BruteForceMode.cpp`

### Step 1: Create `src/BruteForceMode.h`

```cpp
#pragma once
#include "UIScreen.h"
#include "SignalStorage.h"
#include "IRController.h"

namespace BruteForceMode {
  void enter(UIScreen& ui, SignalStorage& storage, IRController& ir);
  void update(UIScreen& ui, SignalStorage& storage, IRController& ir);
  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir);
}
```

### Step 2: Create `src/BruteForceMode.cpp`

Full content:
```cpp
#include "BruteForceMode.h"
#include "PresetNames.h"
#include <M5Unified.h>

namespace {

  enum class BruteState { PROTOCOL_SELECT, BRUTING, SAVE_CONFIRM, DONE };
  BruteState bState = BruteState::PROTOCOL_SELECT;

  const std::vector<std::string> PROTOCOLS = {"NEC", "SAMSUNG", "LG", "SONY", "PANASONIC"};
  int protocolIndex = 0;

  // NEC: 24 addrs x 10 cmds = 240
  const std::vector<uint32_t> NEC_ADDRS = {
    0x0000, 0x00FF, 0x01FE, 0x02FD, 0x04FB,
    0x08F7, 0x10EF, 0x20DF, 0x40BF, 0x807F,
    0xA55A, 0xAD52, 0xBD42, 0xC03F, 0xD02F,
    0xE01F, 0xE0E0, 0xE110, 0xE190, 0xE210,
    0xE290, 0xE310, 0xE390, 0xFF00
  };
  const std::vector<uint32_t> NEC_CMDS = {
    0x02, 0x07, 0x09, 0x0C, 0x10,
    0x1E, 0x40, 0x43, 0x44, 0x45
  };

  // Samsung: 20 addrs x 10 cmds = 200
  const std::vector<uint32_t> SAMSUNG_ADDRS = {
    0xE0E0, 0xE110, 0xE190, 0xE210, 0xE290,
    0xE310, 0xE390, 0xE410, 0xE490, 0xE510,
    0xE590, 0xE610, 0xE690, 0xE710, 0xE790,
    0xE810, 0xE890, 0xE910, 0xE990, 0xEA10
  };
  const std::vector<uint32_t> SAMSUNG_CMDS = {
    0x02, 0x07, 0x0B, 0x0C, 0x0D,
    0x1E, 0x40, 0x43, 0x44, 0x45
  };

  // LG: 16 addrs x 10 cmds = 160
  const std::vector<uint32_t> LG_ADDRS = {
    0x20DF, 0x2FD4, 0x2FD5, 0x2FD6, 0x2FD7,
    0x2FD8, 0x2FD9, 0x2FDA, 0x2FDB, 0x2FDC,
    0x2FDD, 0x2FDE, 0x2FDF, 0x2FE0, 0x2FE1,
    0x2FE2
  };
  const std::vector<uint32_t> LG_CMDS = {
    0x10, 0x14, 0x15, 0x16, 0x17,
    0x43, 0x44, 0x45, 0x58, 0x5C
  };

  // Sony: 16 addrs x 10 cmds = 160
  const std::vector<uint32_t> SONY_ADDRS = {
    0x0000, 0x0001, 0x0010, 0x0011, 0x0100,
    0x0101, 0x0110, 0x0111, 0x1000, 0x1001,
    0x1010, 0x1011, 0x1100, 0x1101, 0x1110,
    0x1111
  };
  const std::vector<uint32_t> SONY_CMDS = {
    0xA90, 0xA91, 0xA92, 0xA93, 0xA94,
    0x490, 0x491, 0xC90, 0x290, 0x090
  };

  // Panasonic: 8 addrs x 10 cmds = 80
  const std::vector<uint32_t> PANASONIC_ADDRS = {
    0x4004, 0x8008, 0xBFAE, 0xBFAD,
    0xBFAC, 0xBFAB, 0xBFAA, 0xBFA9
  };
  const std::vector<uint32_t> PANASONIC_CMDS = {
    0x0100, 0x0190, 0x0191, 0x0192, 0x0193,
    0x0194, 0x0195, 0x0196, 0x0197, 0x0198
  };

  struct ProtocolData {
    const std::vector<uint32_t>* addrs;
    const std::vector<uint32_t>* cmds;
  };

  ProtocolData getProtocolData(const std::string& protocol) {
    if (protocol == "NEC") return {&NEC_ADDRS, &NEC_CMDS};
    if (protocol == "SAMSUNG") return {&SAMSUNG_ADDRS, &SAMSUNG_CMDS};
    if (protocol == "LG") return {&LG_ADDRS, &LG_CMDS};
    if (protocol == "SONY") return {&SONY_ADDRS, &SONY_CMDS};
    return {&PANASONIC_ADDRS, &PANASONIC_CMDS};
  }

  const std::vector<uint32_t>* currentAddrList = nullptr;
  const std::vector<uint32_t>* currentCmdList = nullptr;
  int addrIndex = 0;
  int cmdIndex = 0;
  int totalCombos = 0;
  int currentCombo = 0;

  unsigned long lastSendTime = 0;
  unsigned long stepIntervalMs = 1000;

  Signal hitSignal;
  int saveCatIndex = 0;
  const std::vector<std::string>* currentNameList = nullptr;

  std::string uint32ToHex(uint32_t value) {
    if (value == 0) return "0x0";
    char buf[16];
    snprintf(buf, sizeof(buf), "0x%04X", value);
    return std::string(buf);
  }

  void drawProtocolSelect(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "[F]FIND");
    ui.drawMenu(PROTOCOLS, protocolIndex, "Pick protocol");
    ui.drawFooter("Short:NXT Long:GO");
  }

  void drawBruting(UIScreen& ui, SignalStorage& storage) {
    std::string name = PROTOCOLS[protocolIndex] + " [" +
                       std::to_string(currentCombo) + "/" +
                       std::to_string(totalCombos) + "]";
    std::string addr = (currentAddrList && addrIndex < static_cast<int>(currentAddrList->size()))
                       ? uint32ToHex((*currentAddrList)[addrIndex]) : "--";
    std::string cmd = (currentCmdList && cmdIndex < static_cast<int>(currentCmdList->size()))
                      ? uint32ToHex((*currentCmdList)[cmdIndex]) : "--";

    ui.drawStatusBar("[B]", storage.getCount(), "[F]FIND");
    ui.drawSignalInfo(name, PROTOCOLS[protocolIndex], addr, cmd, "Trying...");
    ui.drawFooter("Short:FAST Long:HIT");
  }

  void drawDone(UIScreen& ui, SignalStorage& storage) {
    ui.drawStatusBar("[B]", storage.getCount(), "[F]FIND");
    ui.drawSignalInfo("Done", "No match", "", "", "Try another protocol");
    ui.drawFooter("btnB:Back");
  }

  void drawSaveConfirm(UIScreen& ui, SignalStorage& storage) {
    std::vector<std::string> cats = PresetNames::CATEGORIES;
    ui.drawStatusBar("[B]", storage.getCount(), "[F]SAVE");
    ui.drawMenu(cats, saveCatIndex, "Pick category");
    ui.drawFooter("Short:NXT Long:SAVE");
  }

  void advanceCombination() {
    cmdIndex++;
    currentCombo++;
    if (cmdIndex >= static_cast<int>(currentCmdList->size())) {
      cmdIndex = 0;
      addrIndex++;
      if (addrIndex >= static_cast<int>(currentAddrList->size())) {
        bState = BruteState::DONE;
      }
    }
  }

} // namespace

namespace BruteForceMode {

  void enter(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    bState = BruteState::PROTOCOL_SELECT;
    protocolIndex = 0;
    currentAddrList = nullptr;
    currentCmdList = nullptr;
    addrIndex = 0;
    cmdIndex = 0;
    totalCombos = 0;
    currentCombo = 0;
    lastSendTime = 0;
    stepIntervalMs = 1000;
    drawProtocolSelect(ui, storage);
  }

  void update(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (bState == BruteState::BRUTING) {
      if (millis() - lastSendTime >= stepIntervalMs) {
        lastSendTime = millis();

        if (currentAddrList && currentCmdList &&
            addrIndex < static_cast<int>(currentAddrList->size()) &&
            cmdIndex < static_cast<int>(currentCmdList->size())) {

          Signal test;
          test.protocol = PROTOCOLS[protocolIndex];
          test.address = uint32ToHex((*currentAddrList)[addrIndex]);
          test.command = uint32ToHex((*currentCmdList)[cmdIndex]);

          ir.sendSignal(test);
          drawBruting(ui, storage);
          advanceCombination();
        }
      }
    }
  }

  void onShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (bState == BruteState::PROTOCOL_SELECT) {
      protocolIndex = (protocolIndex + 1) % PROTOCOLS.size();
      drawProtocolSelect(ui, storage);
    } else if (bState == BruteState::BRUTING) {
      // toggle fast mode
      stepIntervalMs = (stepIntervalMs == 1000) ? 500 : 1000;
      drawBruting(ui, storage);
    } else if (bState == BruteState::SAVE_CONFIRM) {
      saveCatIndex = (saveCatIndex + 1) % PresetNames::CATEGORIES.size();
      drawSaveConfirm(ui, storage);
    } else if (bState == BruteState::DONE) {
      // no-op
    }
  }

  bool onLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (bState == BruteState::PROTOCOL_SELECT) {
      // Start bruting
      ProtocolData pd = getProtocolData(PROTOCOLS[protocolIndex]);
      currentAddrList = pd.addrs;
      currentCmdList = pd.cmds;
      addrIndex = 0;
      cmdIndex = 0;
      currentCombo = 1;
      totalCombos = static_cast<int>(currentAddrList->size() * currentCmdList->size());
      stepIntervalMs = 1000;
      lastSendTime = millis();
      bState = BruteState::BRUTING;
      drawBruting(ui, storage);
      return false;
    } else if (bState == BruteState::BRUTING) {
      // HIT! Save this code
      hitSignal.protocol = PROTOCOLS[protocolIndex];
      // Step back one to get the code we just sent
      int savedAddrIdx = addrIndex;
      int savedCmdIdx = cmdIndex;
      if (savedCmdIdx > 0) {
        savedCmdIdx--;
      } else if (savedAddrIdx > 0) {
        savedAddrIdx--;
        savedCmdIdx = static_cast<int>(currentCmdList->size()) - 1;
      }
      if (currentAddrList && currentCmdList) {
        hitSignal.address = uint32ToHex((*currentAddrList)[savedAddrIdx]);
        hitSignal.command = uint32ToHex((*currentCmdList)[savedCmdIdx]);
      }
      saveCatIndex = 0;
      bState = BruteState::SAVE_CONFIRM;
      drawSaveConfirm(ui, storage);
      return false;
    } else if (bState == BruteState::SAVE_CONFIRM) {
      // Confirm save
      if (storage.getCount() >= SignalStorage::MAX_SIGNALS) {
        // Show full message briefly, then go back to bruting
        bState = BruteState::BRUTING;
        drawBruting(ui, storage);
        return false;
      }
      std::string cat = PresetNames::CATEGORIES[saveCatIndex];
      hitSignal.name = cat + "-Power";
      hitSignal.category = cat;
      hitSignal.id = -1;
      storage.addSignal(hitSignal);
      bState = BruteState::BRUTING;
      drawBruting(ui, storage);
      return false;
    } else if (bState == BruteState::DONE) {
      return true;  // back to main menu
    }
    return true;
  }

  bool onBtnBShortPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    if (bState == BruteState::PROTOCOL_SELECT) {
      return true;  // back to main menu
    } else if (bState == BruteState::BRUTING) {
      bState = BruteState::PROTOCOL_SELECT;
      drawProtocolSelect(ui, storage);
      return false;
    } else if (bState == BruteState::SAVE_CONFIRM) {
      bState = BruteState::BRUTING;
      drawBruting(ui, storage);
      return false;
    } else if (bState == BruteState::DONE) {
      bState = BruteState::PROTOCOL_SELECT;
      drawProtocolSelect(ui, storage);
      return false;
    }
    return true;
  }

  bool onBtnBLongPress(UIScreen& ui, SignalStorage& storage, IRController& ir) {
    return true;  // always back to main menu
  }

} // namespace BruteForceMode
```

### Step 3: Compile

Run: `pio run -e m5stick-s3`

Expected: Builds successfully. BruteForceMode is not yet wired into AppStateMachine, so it is compiled but unreachable.

### Step 4: Commit

```bash
git add src/BruteForceMode.h src/BruteForceMode.cpp
git commit -m "feat: implement BruteForceMode with 5-protocol enumeration"
```

---

## Task 6: Integrate BruteForceMode into AppStateMachine + Final Verification

**Files:**
- Modify: `src/AppStateMachine.h`
- Modify: `src/AppStateMachine.cpp`

### Step 1: Add BRUTE_FORCE to AppState enum

In `src/AppStateMachine.h`, change:
```cpp
enum class AppState {
  MAIN_MENU,
  SCAN_MODE,
  CONTROL_MODE,
  SIGNAL_MANAGER,
  SETTINGS,
  BRUTE_FORCE
};
```

### Step 2: Add "Find" to main menu

In `src/AppStateMachine.h`, change:
```cpp
  static constexpr const char* MENU_ITEMS[] = {"Scan", "Ctrl", "Mgmt", "Set", "Find"};
  static constexpr int MENU_COUNT = 5;
```

### Step 3: Wire BruteForceMode and its btnB into AppStateMachine

In `src/AppStateMachine.cpp`, add include:
```cpp
#include "BruteForceMode.h"
```

In `changeState()`, add BRUTE_FORCE case:
```cpp
    case AppState::BRUTE_FORCE:
      BruteForceMode::enter(ui_, storage_, ir_);
      break;
```

In `update()` switch, add:
```cpp
    case AppState::BRUTE_FORCE:
      BruteForceMode::update(ui_, storage_, ir_);
      break;
```

In `onBtnAShortPress()` switch, add:
```cpp
    case AppState::BRUTE_FORCE:
      BruteForceMode::onShortPress(ui_, storage_, ir_);
      break;
```

In `onBtnALongPress()` switch, add:
```cpp
    case AppState::BRUTE_FORCE:
      if (BruteForceMode::onLongPress(ui_, storage_, ir_)) {
        changeState(AppState::MAIN_MENU);
      }
      break;
```

In `onBtnBShortPress()` switch, add:
```cpp
    case AppState::BRUTE_FORCE:
      returnToMenu = BruteForceMode::onBtnBShortPress(ui_, storage_, ir_);
      break;
```

In `onBtnBLongPress()` switch, add:
```cpp
    case AppState::BRUTE_FORCE:
      returnToMenu = BruteForceMode::onBtnBLongPress(ui_, storage_, ir_);
      break;
```

### Step 4: Update main menu footer hint

In `drawMainMenu()`, the footer says `"Short:SEL Long:ENTR"`. Since btnB now exists, consider updating to `"A:NXT B:BACK L:ENTR"` or keeping as-is. The spec does not require a footer change — leave it as `"Short:SEL Long:ENTR"` since btnB is self-explanatory (back).

### Step 5: Final compile

Run: `pio run -e m5stick-s3`

Expected: Builds successfully with no errors or warnings.

### Step 6: Commit

```bash
git add src/AppStateMachine.h src/AppStateMachine.cpp
git commit -m "feat: integrate BruteForceMode into AppStateMachine, add Find menu"
```

---

## Self-Review Checklist

**1. Spec coverage:**
- [x] BruteForceMode with protocol selection — Task 5
- [x] Auto-step enumeration (1s interval, 0.5s fast mode) — Task 5
- [x] 5 protocols with predefined address/cmd arrays — Task 5
- [x] Hit save with category selection — Task 5
- [x] btnB short = back one level — Tasks 2, 3, 5
- [x] btnB long = back to main menu — Tasks 2, 3, 5
- [x] All modes handle btnB — Tasks 2, 3, 5
- [x] AppStateMachine dual-button detection — Task 1
- [x] Preset names shared between SignalManager and BruteForceMode — Task 4

**2. Placeholder scan:**
- [x] No "TBD", "TODO", "implement later"
- [x] No vague "add error handling" without specifics
- [x] All function signatures are fully defined
- [x] All code blocks show actual code

**3. Type consistency:**
- [x] `onBtnBShortPress` / `onBtnBLongPress` signatures match across all mode headers
- [x] `BruteForceMode` API matches AppStateMachine dispatch pattern
- [x] `PresetNames::getNameList` returns `const std::vector<std::string>*` consistently
