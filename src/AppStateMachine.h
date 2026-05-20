#pragma once

#include "UIScreen.h"
#include "SignalStorage.h"
#include "IRController.h"

enum class AppState {
  MAIN_MENU = 0,
  CONTROL_MODE = 1,
  MATCH_MODE = 2,
  SIGNAL_MANAGER = 3,
  SETTINGS = 4
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

  static constexpr const char* MENU_ITEMS[] = {"Ctrl", "Match", "Mgmt", "Set"};
  static constexpr int MENU_COUNT = 4;
  int menuIndex_ = 0;

  void handleMainMenu();
  void handleControlMode();
  void handleMatchMode();
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
