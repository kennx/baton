#include "AppStateMachine.h"
#include "ScanMode.h"
#include "ControlMode.h"
#include "SignalManager.h"
#include "SettingsMode.h"

constexpr const char* AppStateMachine::MENU_ITEMS[AppStateMachine::MENU_COUNT];

void AppStateMachine::begin() {
  M5.begin();
  ui_.init();
  storage_.begin();
  ir_.begin();
  drawMainMenu();
}

void AppStateMachine::update() {
  M5.update();
  checkBtnA();
  checkBtnB();

  switch (state_) {
    case AppState::MAIN_MENU:
      handleMainMenu();
      break;
    case AppState::SCAN_MODE:
      handleScanMode();
      break;
    case AppState::CONTROL_MODE:
      handleControlMode();
      break;
    case AppState::SIGNAL_MANAGER:
      handleSignalManager();
      break;
    case AppState::SETTINGS:
      handleSettings();
      break;
  }
}

void AppStateMachine::changeState(AppState newState) {
  state_ = newState;
  ui_.clear();

  switch (state_) {
    case AppState::MAIN_MENU:
      menuIndex_ = 0;
      drawMainMenu();
      break;
    case AppState::SCAN_MODE:
      ScanMode::enter(ui_, storage_, ir_);
      break;
    case AppState::CONTROL_MODE:
      ControlMode::enter(ui_, storage_);
      break;
    case AppState::SIGNAL_MANAGER:
      SignalManager::enter(ui_, storage_);
      break;
    case AppState::SETTINGS:
      SettingsMode::enter(ui_, storage_);
      break;
  }
}

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

void AppStateMachine::onBtnAShortPress() {
  switch (state_) {
    case AppState::MAIN_MENU:
      menuIndex_ = (menuIndex_ + 1) % MENU_COUNT;
      drawMainMenu();
      break;
    case AppState::SCAN_MODE:
      ScanMode::onShortPress(ui_, storage_, ir_);
      break;
    case AppState::CONTROL_MODE:
      ControlMode::onShortPress(ui_, storage_);
      break;
    case AppState::SIGNAL_MANAGER:
      SignalManager::onShortPress(ui_, storage_);
      break;
    case AppState::SETTINGS:
      SettingsMode::onShortPress(ui_, storage_);
      break;
  }
}

void AppStateMachine::onBtnALongPress() {
  switch (state_) {
    case AppState::MAIN_MENU:
      changeState(static_cast<AppState>(static_cast<int>(AppState::SCAN_MODE) + menuIndex_));
      break;
    case AppState::SCAN_MODE:
      if (ScanMode::onLongPress(ui_, storage_, ir_)) {
        changeState(AppState::MAIN_MENU);
      }
      break;
    case AppState::CONTROL_MODE:
      if (ControlMode::onLongPress(ui_, storage_, ir_)) {
        changeState(AppState::MAIN_MENU);
      }
      break;
    case AppState::SIGNAL_MANAGER:
      if (SignalManager::onLongPress(ui_, storage_)) {
        changeState(AppState::MAIN_MENU);
      }
      break;
    case AppState::SETTINGS:
      if (SettingsMode::onLongPress(ui_, storage_)) {
        changeState(AppState::MAIN_MENU);
      }
      break;
  }
}

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

void AppStateMachine::drawMainMenu() {
  std::vector<std::string> items;
  for (int i = 0; i < MENU_COUNT; i++) {
    items.emplace_back(MENU_ITEMS[i]);
  }
  ui_.drawStatusBar("[B]", storage_.getCount(), "MENU");
  ui_.drawMenu(items, menuIndex_);
  ui_.drawFooter("Short:SEL Long:ENTR");
}

void AppStateMachine::handleMainMenu() {
  // MENU在 checkButton 中处理
}

void AppStateMachine::handleScanMode() {
  ScanMode::update(ui_, storage_, ir_);
}

void AppStateMachine::handleControlMode() {
  ControlMode::update(ui_, storage_, ir_);
}

void AppStateMachine::handleSignalManager() {
  SignalManager::update(ui_, storage_);
}

void AppStateMachine::handleSettings() {
  SettingsMode::update(ui_, storage_);
}
