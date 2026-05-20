#pragma once

#include <M5Unified.h>
#include <vector>
#include <string>
#include <functional>

class UIScreen {
public:
  static constexpr int SCREEN_WIDTH = 135;
  static constexpr int SCREEN_HEIGHT = 240;
  static constexpr int STATUS_BAR_H = 24;
  static constexpr int FOOTER_H = 24;
  static constexpr int CONTENT_Y = STATUS_BAR_H;
  static constexpr int CONTENT_H = SCREEN_HEIGHT - STATUS_BAR_H - FOOTER_H;

  void init();
  void clear();
  void drawStatusBar(const std::string& battery, int signalCount, const std::string& mode);
  void drawFooter(const std::string& hint);
  void drawMenu(const std::vector<std::string>& items, int selectedIndex, const std::string& title = "");
  void drawMenu(int totalItems, int selectedIndex, std::function<std::string(int)> getItemText, const std::string& title = "");
  void drawPopup(const std::string& title, const std::string& message, const std::vector<std::string>& options, int selectedOption);
  void drawSignalInfo(const std::string& name, const std::string& protocol,
                      const std::string& address, const std::string& command,
                      const std::string& status);
  void drawProgressBar(const std::string& label, int percent);
  void setBrightness(uint8_t percent);

private:
  void drawBackground();
  void drawCenteredText(const std::string& text, int y);
  void drawText(const std::string& text, int x, int y);
  void fillRect(int x, int y, int w, int h, uint32_t color);
};
