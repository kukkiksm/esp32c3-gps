#include "oled.h"
#include <U8g2lib.h>
#include <Wire.h>

#ifdef OLED_USE_128
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#elif defined(OLED_USE_72)
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#else
#error "Please define either OLED_USE_128 or OLED_USE_72"
#endif

void oledInit() {
  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();
  u8g2.setContrast(200);
}

void oledPrint(String line1, String line2, String line3, String line4) {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_profont10_tr);
#ifdef OLED_USE_128
  u8g2.drawStr(0, 10, line1.c_str());
#else
  u8g2.drawStr(0, 9, line1.c_str());
#endif

  u8g2.setFont(u8g2_font_6x10_tr);
#ifdef OLED_USE_128
  if (line2.length() > 0) u8g2.drawStr(0, 24, line2.c_str());
  if (line3.length() > 0) u8g2.drawStr(0, 38, line3.c_str());
  if (line4.length() > 0) u8g2.drawStr(0, 52, line4.c_str());
#else
  if (line2.length() > 0) u8g2.drawStr(0, 20, line2.c_str());
  if (line3.length() > 0) u8g2.drawStr(0, 30, line3.c_str());
  if (line4.length() > 0) u8g2.drawStr(0, 39, line4.c_str());
#endif

  u8g2.sendBuffer();
}
