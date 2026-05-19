#pragma once

#include "UIScreen.h"
#include "SignalStorage.h"
#include "IRController.h"

enum class AppState {
  MAIN_MENU,
  SCAN_MODE,
  CONTROL_MODE,
  SIGNAL_MANAGER,
  SETTINGS
};

class AppStateMachine {
public:
  static constexpr unsigned long LONG_PRESS_MS = 500;
  static constexpr unsigned long DEBOUNCE_MS = 50;

  void begin();
  void update();
  void changeState(AppState newState);
  AppState getState() const { return state_; }

private:
  AppState state_ = AppState::MAIN_MENU;
  UIScreen ui_;
  SignalStorage storage_;
  IRController ir_;

  bool btnWasPressed_ = false;
  unsigned long btnPressStart_ = 0;
  bool longPressHandled_ = false;

  static constexpr const char* MENU_ITEMS[] = {"扫描模式", "控制模式", "信号管理", "设置"};
  static constexpr int MENU_COUNT = 4;
  int menuIndex_ = 0;

  void handleMainMenu();
  void handleScanMode();
  void handleControlMode();
  void handleSignalManager();
  void handleSettings();

  void checkButton();
  bool isShortPress();
  bool isLongPress();
  void drawMainMenu();
};
