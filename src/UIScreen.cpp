#include "UIScreen.h"

// Define modern colors
// IMPORTANT: Use uint16_t for RGB565 values. uint32_t is interpreted as RGB888 by M5GFX.
static constexpr uint16_t COLOR_BG = 0x10A2; // RGB565(16,16,16) roughly
static constexpr uint16_t COLOR_DOT = 0x2124; // RGB565(32,32,32)
static constexpr uint16_t COLOR_ACCENT = 0x03DF; // Bright Blue matching the button (RGB: 0, 122, 255)
static constexpr uint16_t COLOR_TEXT = TFT_WHITE;
static constexpr uint16_t COLOR_HEADER = 0x2104; // Darker grey

void UIScreen::init() {
  M5.Display.setRotation(0);  // 135x240
  M5.Display.setFont(&fonts::FreeSans9pt7b);
  M5.Display.setTextSize(1);
  clear();
}

void UIScreen::drawBackground() {
  M5.Display.fillScreen(COLOR_BG);
  // Draw 5x5 dot matrix
  for (int x = 2; x < SCREEN_WIDTH; x += 5) {
    for (int y = 2; y < SCREEN_HEIGHT; y += 5) {
      M5.Display.drawPixel(x, y, COLOR_DOT);
    }
  }
}

void UIScreen::clear() {
  drawBackground();
}

void UIScreen::drawStatusBar(const std::string& battery, int signalCount, const std::string& mode) {
  fillRect(0, 0, SCREEN_WIDTH, STATUS_BAR_H, COLOR_BG);
  M5.Display.setFont(&fonts::FreeSans9pt7b);
  M5.Display.setTextColor(COLOR_TEXT, COLOR_BG);
  int y = (STATUS_BAR_H - M5.Display.fontHeight()) / 2;
  M5.Display.setCursor(4, y);
  M5.Display.printf("%s %d %s", battery.c_str(), signalCount, mode.c_str());
}

void UIScreen::drawFooter(const std::string& hint) {
  int y = SCREEN_HEIGHT - FOOTER_H;
  fillRect(0, y, SCREEN_WIDTH, FOOTER_H, COLOR_BG);
  M5.Display.setFont(&fonts::FreeSans9pt7b);
  M5.Display.setTextColor(COLOR_TEXT, COLOR_BG);
  int textY = y + (FOOTER_H - M5.Display.fontHeight()) / 2;
  drawCenteredText(hint, textY);
}

void UIScreen::drawMenu(int totalItems, int selectedIndex, std::function<std::string(int)> getItemText, const std::string& title) {
  // Clear content area with background
  fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_H, COLOR_BG);
  // Redraw dots in content area
  for (int x = 2; x < SCREEN_WIDTH; x += 5) {
    for (int y = CONTENT_Y + 2; y < CONTENT_Y + CONTENT_H; y += 5) {
      M5.Display.drawPixel(x, y, COLOR_DOT);
    }
  }

  M5.Display.setFont(&fonts::FreeSans9pt7b);

  // Use font height from M5GFX for accurate metrics
  const int fontH = M5.Display.fontHeight();
  const int lineHeight = fontH + 6; // font height + vertical padding
  int topY = CONTENT_Y + 4;

  if (!title.empty()) {
    M5.Display.setTextColor(COLOR_ACCENT, COLOR_BG);
    M5.Display.setTextDatum(top_center);
    M5.Display.drawString(title.c_str(), SCREEN_WIDTH / 2, topY);
    topY += lineHeight;
    M5.Display.setTextDatum(top_left); // restore default
  }

  int maxVisible = (CONTENT_Y + CONTENT_H - topY) / lineHeight;
  
  int startIdx = selectedIndex - maxVisible / 2;
  if (startIdx < 0) startIdx = 0;
  if (startIdx + maxVisible > totalItems) {
    startIdx = totalItems - maxVisible;
    if (startIdx < 0) startIdx = 0;
  }

  // Use middle_left datum: drawString Y = vertical center of text
  M5.Display.setTextDatum(middle_left);

  for (int i = 0; i < maxVisible && (startIdx + i) < totalItems; i++) {
    int idx = startIdx + i;
    int itemY = topY + i * lineHeight;      // top of this line slot
    int centerY = itemY + lineHeight / 2;    // vertical center

    if (idx == selectedIndex) {
      M5.Display.fillRect(0, itemY, SCREEN_WIDTH, lineHeight, COLOR_ACCENT);
      M5.Display.setTextColor(TFT_BLACK, COLOR_ACCENT);
    } else {
      M5.Display.setTextColor(COLOR_TEXT, COLOR_BG);
    }
    
    // Draw text (limit length to prevent wrapping)
    std::string text = getItemText(idx);
    if (text.length() > 14) text = text.substr(0, 12) + "..";
    
    M5.Display.drawString(text.c_str(), 8, centerY);
  }

  // Restore default datum
  M5.Display.setTextDatum(top_left);
}

void UIScreen::drawMenu(const std::vector<std::string>& items, int selectedIndex, const std::string& title) {
  drawMenu(items.size(), selectedIndex, [&items](int idx) { return items[idx]; }, title);
}

void UIScreen::drawPopup(const std::string& title, const std::string& message,
                         const std::vector<std::string>& options, int selectedOption) {
  int pw = SCREEN_WIDTH - 20;
  int ph = 100;
  int px = 10;
  int py = (SCREEN_HEIGHT - ph) / 2;

  M5.Display.fillRoundRect(px, py, pw, ph, 8, COLOR_HEADER);
  M5.Display.drawRoundRect(px, py, pw, ph, 8, COLOR_ACCENT);

  M5.Display.setFont(&fonts::FreeSans9pt7b);
  M5.Display.setTextColor(COLOR_ACCENT, COLOR_HEADER);
  drawCenteredText(title, py + 20);

  M5.Display.setTextColor(COLOR_TEXT, COLOR_HEADER);
  drawCenteredText(message, py + 44);

  int oy = py + 64;
  for (size_t i = 0; i < options.size(); i++) {
    if (static_cast<int>(i) == selectedOption) {
      M5.Display.fillRect(px + 10, oy - 14, pw - 20, 20, COLOR_ACCENT);
      M5.Display.setTextColor(TFT_BLACK, COLOR_ACCENT);
    } else {
      M5.Display.setTextColor(COLOR_TEXT, COLOR_HEADER);
    }
    drawCenteredText(options[i], oy);
    oy += 24;
  }
}

void UIScreen::drawSignalInfo(const std::string& name, const std::string& protocol,
                              const std::string& address, const std::string& command,
                              const std::string& status) {
  fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_H, COLOR_BG);
  // Redraw dots
  for (int x = 2; x < SCREEN_WIDTH; x += 5) {
    for (int y = CONTENT_Y + 2; y < CONTENT_Y + CONTENT_H; y += 5) {
      M5.Display.drawPixel(x, y, COLOR_DOT);
    }
  }

  int y = CONTENT_Y + 24;
  M5.Display.setFont(&fonts::FreeSans9pt7b);
  
  M5.Display.setTextColor(COLOR_TEXT, COLOR_BG);
  drawCenteredText(name, y);
  y += 24;

  M5.Display.setTextColor(TFT_LIGHTGREY, COLOR_BG);
  drawCenteredText(protocol, y);
  y += 24;
  drawCenteredText("Addr: " + address, y);
  y += 24;
  drawCenteredText("Cmd: " + command, y);
  y += 32;

  if (status == "SENDING...") {
    M5.Display.setTextColor(TFT_RED, COLOR_BG);
  } else {
    M5.Display.setTextColor(TFT_GREEN, COLOR_BG);
  }
  drawCenteredText(status, y);
}

void UIScreen::drawProgressBar(const std::string& label, int percent) {
  fillRect(0, CONTENT_Y, SCREEN_WIDTH, CONTENT_H, COLOR_BG);

  int y = CONTENT_Y + CONTENT_H / 2 - 20;
  M5.Display.setFont(&fonts::FreeSans9pt7b);
  M5.Display.setTextColor(COLOR_TEXT, COLOR_BG);
  drawCenteredText(label, y);

  int barY = y + 10;
  int barW = SCREEN_WIDTH - 20;
  int barH = 12;
  int barX = 10;

  M5.Display.drawRoundRect(barX, barY, barW, barH, 4, COLOR_TEXT);
  int fillW = (barW - 4) * percent / 100;
  if (fillW > 0) {
    M5.Display.fillRoundRect(barX + 2, barY + 2, fillW, barH - 4, 2, COLOR_ACCENT);
  }
}

void UIScreen::setBrightness(uint8_t percent) {
  uint8_t level = percent > 100 ? 255 : (percent * 255 / 100);
  M5.Display.setBrightness(level);
}

void UIScreen::drawCenteredText(const std::string& text, int y) {
  int16_t cx = (SCREEN_WIDTH - M5.Display.textWidth(text.c_str())) / 2;
  M5.Display.setCursor(cx > 0 ? cx : 0, y);
  M5.Display.print(text.c_str());
}

void UIScreen::drawText(const std::string& text, int x, int y) {
  M5.Display.setCursor(x, y);
  M5.Display.print(text.c_str());
}

void UIScreen::fillRect(int x, int y, int w, int h, uint16_t color) {
  M5.Display.fillRect(x, y, w, h, color);
}
