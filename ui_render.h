#pragma once

#include <M5Core2.h>

/* Draw centered text */
inline void showText(const String& text, uint8_t size) {
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextFont(4);
  M5.Lcd.setTextSize(size);
  M5.Lcd.setTextColor(BLACK);

  int16_t textWidth = M5.Lcd.textWidth(text);
  int16_t x = (M5.Lcd.width() - textWidth) / 2;
  int16_t y = (M5.Lcd.height() - M5.Lcd.fontHeight()) / 2;

  M5.Lcd.setCursor(x, y);
  M5.Lcd.print(text);
}
