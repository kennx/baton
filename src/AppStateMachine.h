#pragma once

#include "UIScreen.h"
#include "SignalStorage.h"
#include "IRController.h"

enum class AppState {
  MAIN_MENU,
  SCAN_MODE,
  CONTROL_MODE,
  SIGNAL_MANAGER,
  SETTINGS,
  BRUTE_FORCE
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

  bool btnBWasPressed_ = false;
  unsigned long btnBPressStart_ = 0;
  bool btnBLongHandled_ = false;

  static constexpr const char* MENU_ITEMS[] = {"Scan", "Ctrl", "Mgmt", "Set", "Find"};
  static constexpr int MENU_COUNT = 5;
  int menuIndex_ = 0;

  void handleMainMenu();
  void handleScanMode();
  void handleControlMode();
  void handleSignalManager();
  void handleSettings();

  void checkBtnA();
  void checkBtnB();
  void onBtnAShortPress();
  void onBtnALongPress();
  void onBtnBShortPress();
  void onBtnBLongPress();
  void drawMainMenu();
};
