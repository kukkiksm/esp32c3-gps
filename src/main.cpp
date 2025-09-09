#include <WiFi.h>
#include <esp_now.h>
#include "gps.h"
#include "config.h"
#include <LittleFS.h>
#include "oled.h"
#include <esp_wifi.h>
#include <ledsignal.h>
#include <OneButton.h>
#include "oled.h"
#include "gps.h"

#define BOOT_PIN 9
#define GPS_RX_PIN 20 // GPS (TX) --> ESP32 GPIO20 (RX)
#define GPS_TX_PIN 21 // GPS (RX) <-- ESP32 GPIO21 (TX)

OneButton button(BOOT_PIN, true, true);
gps_data_t gpsData;

int gpsViewMode = 0;
unsigned long lastToggle = 0;

unsigned long bootPressStart = 0;
bool bootHeld = false;

String getGpsInfoStr()
{
  switch (gpsViewMode)
  {
  case 0:
    return "SAT " + String(gpsData.sats);
  case 1:
    return "ALT " + String(gpsData.alt, 1);
  case 2:
    return "SAT " + String(gpsData.sats);
  default:
    return "";
  }
}

String formatSmartCoord(String label, double value)
{
  String valStr = String(value, 5);
  int totalLen = label.length() + 1 + valStr.length(); 

  while (totalLen > 12 && label.length() > 0)
  {
    label.remove(label.length() - 1); 
    totalLen = label.length() + 1 + valStr.length();
  }

  return label + " " + valStr;
}

void printChannelDebug()
{
  uint8_t chan;
  wifi_second_chan_t second;
  esp_wifi_get_channel(&chan, &second);
  Serial.printf("📶 Current Channel = %d\n", chan);
}

void setup()
{
  Serial.begin(115200);
  delay(5000);
  Serial.println("test");

  pinMode(BOOT_PIN, INPUT_PULLUP);

  initLedSignal(8);      
  setLedOnline();        
  digitalWrite(8, HIGH); 

  button.attachClick([]()
                     {
  Serial.println("🖱️ BOOT กดสั้น → toggle morse mode");
  updateMorseMessage(gpsData.lat, gpsData.lon);
  toggleLedMorseMode(); });

  button.attachLongPressStart([]()
                              {
  Serial.println("🧹 BOOT ค้าง > 3 วิ → ล้าง config และรีสตาร์ต...");
  LittleFS.begin(true);
  LittleFS.remove("/config.json");
  delay(500);
  ESP.restart(); });

  oledInit();
  oledPrint("Waiting GPS.....", "", "", "");

  loadConfig(); 

  if (config.receiverMacStr.length() != 17)
  {
    Serial.println("🚫 Invalid MAC. Starting portal...");
    setLedOffline();
    startPortal();
    while (1)
    {
      updateLedSignal();
      delay(50);
    }
  }

  if (WiFi.status() != WL_CONNECTED)
    setLedOffline();
  else
    setLedOnline();

  WiFi.mode(WIFI_STA);                             
  esp_wifi_set_promiscuous(true);                  
  esp_wifi_set_channel(10, WIFI_SECOND_CHAN_NONE); 
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("❌ ESP-NOW Init Failed");
    while (1)
    {
      updateLedSignal();
      delay(50);
    }
  }

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, config.receiverMAC, 6);
  peer.channel = 10;
  peer.encrypt = false;

  if (esp_now_add_peer(&peer) == ESP_OK)
    Serial.println("✅ Peer added");
  else
    Serial.println("❌ Failed to add peer");

  gpsInit(GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("✅ GPS + ESP-NOW Ready (Offline Mode)");

  digitalWrite(8, HIGH); 
}

#include <time.h>

unsigned long lastSend = 0;
bool gpsReadyToSend = false;

void loop()
{
  button.tick();    
  updateLedMorse(); 

  if (millis() - lastToggle > 5000)
  {
    gpsViewMode = (gpsViewMode + 1) % 3;
    lastToggle = millis();
  }

  if (!gpsReadyToSend && gpsRead(gpsData))
  {
    gpsReadyToSend = true;
    lastSend = millis(); 
  }

  if (gpsReadyToSend && millis() - lastSend > 3000)
  {
    gpsReadyToSend = false;

    Serial.println("✅ Got GPS, sending...");

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             config.receiverMAC[0], config.receiverMAC[1], config.receiverMAC[2],
             config.receiverMAC[3], config.receiverMAC[4], config.receiverMAC[5]);
    Serial.printf("📤 Sending to: %s\n", macStr);

    Serial.printf("📍 Lat: %.5f, Lon: %.5f\n", gpsData.lat, gpsData.lon);
    Serial.printf("📏 Alt: %.2f m, 🛰️ Sats: %d\n", gpsData.alt, gpsData.sats);
    Serial.printf("📅 Date: %02d/%02d/%02d 🕒 Time: %02d:%02d:%02d\n",
                  (int)gpsData.day, (int)gpsData.month, (int)(gpsData.year % 100),
                  (int)gpsData.hh, (int)gpsData.mm, (int)gpsData.ss);

    esp_err_t result = esp_now_send(config.receiverMAC, (uint8_t *)&gpsData, sizeof(gpsData));
    Serial.println(result == ESP_OK ? "📡 Sent successfully." : "❌ Send failed!");

    int offset = getCurrentUTCOffset(); 
    struct tm t;
    t.tm_year = gpsData.year - 1900;
    t.tm_mon = gpsData.month - 1;
    t.tm_mday = gpsData.day;
    t.tm_hour = gpsData.hh;
    t.tm_min = gpsData.mm;
    t.tm_sec = gpsData.ss;
    t.tm_isdst = 0;

    time_t raw = mktime(&t); 
    raw += offset * 1800;    

    struct tm *local = localtime(&raw);

    char buf[24];
    sprintf(buf, "%02d/%02d/%02d %02d:%02d",
            local->tm_mday, local->tm_mon + 1, local->tm_year % 100,
            local->tm_hour, local->tm_min);
    String dateTime = String(buf);

    String latStr = formatSmartCoord("LAT", gpsData.lat);
    String lonStr = formatSmartCoord("LON", gpsData.lon);
    String infoStr = getGpsInfoStr();

    oledPrint(dateTime, latStr, lonStr, infoStr);
  }

  delay(10); 

