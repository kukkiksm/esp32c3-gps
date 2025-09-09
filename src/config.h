#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

struct Config {
  String ssid;
  String pass;
  String receiverMacStr;
  uint8_t receiverMAC[6];

  int utcOffsetIdx = 15;  // ✅ default UTC+7 (Bangkok)
};

extern Config config;

bool parseMacString(const String &macStr, uint8_t *macBytes);
void loadConfig();
void saveConfig();
void startPortal();

// ✅ เพิ่มฟังก์ชัน timezone selector
int getCurrentUTCOffset();
int getTotalUTCOffsets();
String webUtcOffsetSelector();

#endif

