#pragma once

#include <M5Core2.h>

extern String bleName;
extern String challenge;

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

inline void showOrderAmount(String label, int qrSize = 200) {
  M5.Lcd.fillScreen(WHITE);

  int screenWidth = M5.Lcd.width();
  
  // Center X positions
  int qrX = (screenWidth - qrSize) / 2;
  int qrY = 10;

  // Draw QR code centered
  String qrData = String(bleName) + "/" + String(challenge);
  M5.Lcd.qrcode(qrData.c_str(), qrX, qrY, qrSize);

  // Set up text
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Lcd.setFreeFont(NULL);
  M5.Lcd.setTextSize(2);

  // Calculate text width manually (approx: each character ~12 pixels wide at textSize(2))
  int textWidth = label.length() * 12; 

  // Draw text centered horizontally below QR
  int textX = (screenWidth - textWidth) / 2;
  int textY = qrY + qrSize + 5; // 5 pixels below QR

  M5.Lcd.setCursor(textX, textY);
  M5.Lcd.print(label);  
}
