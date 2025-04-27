#pragma once

#include <M5Core2.h>

/* Draw centered text */
inline void showText(const String& text, uint8_t size = 1, uint16_t textColor = TFT_BLACK) {
  M5.Lcd.fillScreen(WHITE);

  // Set FreeFont instead of default bitmap font
  M5.Lcd.setFreeFont(&FreeSansBold12pt7b);
  M5.Lcd.setTextSize(size); // Optional scaling
  M5.Lcd.setTextColor(textColor, WHITE);

  int16_t textWidth = M5.Lcd.textWidth(text);
  int16_t x = (M5.Lcd.width() - textWidth) / 2;
  int16_t y = (M5.Lcd.height() - M5.Lcd.fontHeight()) / 2 + M5.Lcd.fontHeight();  // move down by height

  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(text);
}
