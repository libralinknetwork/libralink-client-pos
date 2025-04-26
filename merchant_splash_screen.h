#pragma once
#include <M5Core2.h>

void drawSplashScreen() {
  M5.Lcd.fillScreen(TFT_WHITE);

  // Tent roof
  M5.Lcd.fillTriangle(
    60, 80,    // left
    160, 20,   // top
    260, 80,   // right
    TFT_RED
  );

  // Stall body
  M5.Lcd.fillRect(80, 80, 160, 80, TFT_MAROON);

  // Support poles
  M5.Lcd.fillRect(85, 80, 5, 90, TFT_BLACK);
  M5.Lcd.fillRect(230, 80, 5, 90, TFT_BLACK);

  // Static Merchant Text
  M5.Lcd.setTextColor(TFT_WHITE, TFT_MAROON);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString("Merchant Place", 160, 110, 4);  // Slightly higher

  // Dynamic WiFi AP Name
  M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Lcd.setTextDatum(MC_DATUM);
  M5.Lcd.drawString("Libralink", 160, 210, 2);    // Near bottom, smaller font
}
