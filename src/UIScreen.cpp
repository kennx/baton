#include "UIScreen.h"

void UIScreen::init() {
  M5.Display.setRotation(0);  // 纵向 135x240
  M5.Display.setTextFont(2);  // 默认字体
  M5.Display.setTextSize(1);
  clear();
}

void UIScreen::clear() {
  M5.Display.fillScreen(TFT_BLACK);
}

void UIScreen::drawStatusBar(const std::string& battery, int signalCount, const std::string& mode) {
  fillRect(0, 0, SCREEN_WIDTH, STATUS_BAR_H, TFT_DARKGREY);
  M5.Display.setTextColor(TFT_WHITE, TFT_DARKGREY);
  M5.Display.setCursor(2, 2);
  M5.Display.printf("%s %d %s", battery.c_str(), signalCount, mode.c_str());
}

void UIScreen::drawFooter(const std::string& hint) {
  int y = SCREEN_HEIGHT - FOOTER_H;
  fillRect(0, y, SCREEN_WIDTH, FOOTER_H, TFT_DARKGREY);
  M5.Display.setTextColor(TFT_WHITE, TFT_DARKGREY);
  drawCenteredText(hint, y + 2);
}

void UIScreen::drawMenu(const std::vector<std::string>& items, int selectedIndex, const std::string& title) {
  fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_H, TFT_BLACK);

  int y = CONTENT_Y + 4;

  if (!title.empty()) {
    M5.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    drawCenteredText(title, y);
    y += 18;
  }

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  const int lineHeight = 16;

  for (size_t i = 0; i < items.size() && y < CONTENT_Y + CONTENT_H - lineHeight; i++) {
    if (static_cast<int>(i) == selectedIndex) {
      fillRect(0, y, SCREEN_WIDTH, lineHeight, TFT_DARKGREEN);
      M5.Display.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    } else {
      M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    }
    M5.Display.setCursor(4, y + 2);
    M5.Display.printf("> %s", items[i].c_str());
    y += lineHeight;
  }
}

void UIScreen::drawPopup(const std::string& title, const std::string& message,
                         const std::vector<std::string>& options, int selectedOption) {
  int pw = SCREEN_WIDTH - 10;
  int ph = 80;
  int px = 5;
  int py = (SCREEN_HEIGHT - ph) / 2;

  fillRect(px, py, pw, ph, TFT_NAVY);
  M5.Display.drawRect(px, py, pw, ph, TFT_WHITE);

  M5.Display.setTextColor(TFT_YELLOW, TFT_NAVY);
  drawCenteredText(title, py + 4);

  M5.Display.setTextColor(TFT_WHITE, TFT_NAVY);
  drawCenteredText(message, py + 22);

  int oy = py + 44;
  for (size_t i = 0; i < options.size(); i++) {
    if (static_cast<int>(i) == selectedOption) {
      fillRect(px + 4, oy, pw - 8, 14, TFT_DARKGREEN);
      M5.Display.setTextColor(TFT_WHITE, TFT_DARKGREEN);
    } else {
      M5.Display.setTextColor(TFT_WHITE, TFT_NAVY);
    }
    drawCenteredText(options[i], oy + 2);
    oy += 16;
  }
}

void UIScreen::drawSignalInfo(const std::string& name, const std::string& protocol,
                              const std::string& address, const std::string& command,
                              const std::string& status) {
  fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_H, TFT_BLACK);

  int y = CONTENT_Y + 8;
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);

  drawCenteredText(name, y);
  y += 18;
  drawCenteredText(protocol, y);
  y += 18;
  drawCenteredText("Addr: " + address, y);
  y += 18;
  drawCenteredText("Cmd: " + command, y);
  y += 24;

  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  drawCenteredText(status, y);
}

void UIScreen::drawProgressBar(const std::string& label, int percent) {
  fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_H, TFT_BLACK);

  int y = CONTENT_Y + CONTENT_H / 2 - 20;
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  drawCenteredText(label, y);

  int barY = y + 20;
  int barW = SCREEN_WIDTH - 20;
  int barH = 10;
  int barX = 10;

  M5.Display.drawRect(barX, barY, barW, barH, TFT_WHITE);
  int fillW = (barW - 2) * percent / 100;
  fillRect(barX + 1, barY + 1, fillW, barH - 2, TFT_GREEN);
}

void UIScreen::setBrightness(uint8_t percent) {
  uint8_t level = percent > 100 ? 255 : (percent * 255 / 100);
  M5.Display.setBrightness(level);
}

void UIScreen::drawCenteredText(const std::string& text, int y, uint32_t color) {
  M5.Display.setTextColor(color);
  int16_t cx = (SCREEN_WIDTH - M5.Display.textWidth(text.c_str())) / 2;
  M5.Display.setCursor(cx > 0 ? cx : 0, y);
  M5.Display.print(text.c_str());
}

void UIScreen::drawText(const std::string& text, int x, int y, uint32_t color) {
  M5.Display.setTextColor(color);
  M5.Display.setCursor(x, y);
  M5.Display.print(text.c_str());
}

void UIScreen::fillRect(int x, int y, int w, int h, uint32_t color) {
  M5.Display.fillRect(x, y, w, h, color);
}
