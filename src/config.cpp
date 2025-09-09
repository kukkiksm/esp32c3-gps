#include "config.h"
#include "oled.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

Config config;

bool parseMacString(const String &macStr, uint8_t *macBytes)
{
  int values[6];
  if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
             &values[0], &values[1], &values[2],
             &values[3], &values[4], &values[5]) != 6)
  {
    return false;
  }
  for (int i = 0; i < 6; ++i)
  {
    macBytes[i] = (uint8_t)values[i];
  }
  return true;
}

void loadConfig()
{
  if (!LittleFS.begin(true))
    return;
  File f = LittleFS.open("/config.json", "r");
  if (!f)
    return;
  DynamicJsonDocument doc(256);
  if (deserializeJson(doc, f))
    return;
  config.receiverMacStr = doc["mac"] | "";
  config.utcOffsetIdx = doc["utcoffset"] | 15;
  parseMacString(config.receiverMacStr, config.receiverMAC);
  f.close();
}

void saveConfig()
{
  if (!LittleFS.begin(true))
    return;
  File f = LittleFS.open("/config.json", "w");
  if (!f)
    return;
  DynamicJsonDocument doc(256);
  doc["mac"] = config.receiverMacStr;
  doc["utcoffset"] = config.utcOffsetIdx;
  serializeJson(doc, f);
  f.close();
}

AsyncWebServer server(80);
IPAddress localIP(10, 1, 1, 1);
IPAddress gateway(10, 1, 1, 1);
IPAddress subnet(255, 255, 255, 0);

void startPortal()
{
  String apName = "gps-" + String(ESP.getEfuseMac() & 0xFFFF, HEX);
  oledPrint("Setup Mode", apName, "10.1.1.1", "");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req)
            {
    String html = R"rawliteral(
<!DOCTYPE html><html><head><title>GPS Module Settings</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  body {
    font-family: Arial, sans-serif;
    background: #f4f4f4;
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100vh;
    margin: 0;
  }
  .form-container {
    background: #fff;
    padding: 24px 32px;
    border-radius: 10px;
    box-shadow: 0 0 12px rgba(0,0,0,0.1);
    max-width: 400px;
    width: 100%;
  }
  h2 {
    text-align: center;
    margin-bottom: 24px;
    color: #333;
  }
  fieldset {
    border: 1px solid #ccc;
    padding: 20px;
    border-radius: 6px;
    margin-bottom: 16px;
  }
  legend {
    font-weight: bold;
    padding: 0 10px;
    color: #333;
  }
  label {
    display: block;
    margin-bottom: 6px;
    font-weight: bold;
  }
  input[type="text"], select {
    width: 100%;
    padding: 10px;
    margin-bottom: 16px;
    border: 1px solid #ccc;
    border-radius: 6px;
    font-size: 15px;
  }
  input[type="submit"] {
    width: 100%;
    padding: 12px;
    background: #4CAF50;
    border: none;
    color: white;
    font-size: 16px;
    border-radius: 6px;
    cursor: pointer;
  }
  input[type="submit"]:hover {
    background: #45a049;
  }
</style>
</head><body>
  <div class="form-container">
    <h2>GPS Module Settings</h2>
    <form action="/save" method="POST">
      <fieldset>
        <legend>ATS Mini MAC Address</legend>
        <label for="mac">Receiver MAC</label>
        <input name="mac" type="text" placeholder="24:6F:28:AB:CD:EF" value=")rawliteral";

    if (config.receiverMacStr.length() > 0) {
      html += config.receiverMacStr;
    }

    html += R"rawliteral(">
      </fieldset>

      <fieldset>
        <legend>Time Zone</legend>
        <label for="utcoffset">Select Time Zone</label>
        <select name="utcoffset">
)rawliteral";

    html += webUtcOffsetSelector();

    html += R"rawliteral(
        </select>
      </fieldset>

      <input type="submit" value="Save">
    </form>
  </div>
</body></html>
)rawliteral";

    req->send(200, "text/html", html); });

  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *req)
            {
    config.receiverMacStr = req->getParam("mac", true)->value();
    parseMacString(config.receiverMacStr, config.receiverMAC);

    if (req->hasParam("utcoffset", true)) {
      config.utcOffsetIdx = req->getParam("utcoffset", true)->value().toInt();
    }

    saveConfig();
    req->send(200, "text/html", "<h3>Saved. Rebooting...</h3>");
    delay(2000);
    ESP.restart(); });

  WiFi.softAPConfig(localIP, gateway, subnet);
  WiFi.softAP(("gps-" + String(ESP.getEfuseMac() & 0xFFFF, HEX)).c_str());
  server.begin();
}

struct UTCOffset
{
  int offset;       
  const char *desc; 
  const char *city; 
};

const UTCOffset utcOffsets[] = {
    {-8 * 2, "UTC-8", "Fairbanks"},
    {-7 * 2, "UTC-7", "San Francisco"},
    {-6 * 2, "UTC-6", "Denver"},
    {-5 * 2, "UTC-5", "Houston"},
    {-4 * 2, "UTC-4", "New York"},
    {-3 * 2, "UTC-3", "Rio de Janeiro"},
    {-2 * 2, "UTC-2", "Sandwich Islands"},
    {-1 * 2, "UTC-1", "Nuuk"},
    {0 * 2, "UTC+0", "Reykjavik"},
    {1 * 2, "UTC+1", "London"},
    {2 * 2, "UTC+2", "Berlin"},
    {3 * 2, "UTC+3", "Moscow"},
    {4 * 2, "UTC+4", "Yerevan"},
    {5 * 2, "UTC+5", "Astana"},
    {6 * 2, "UTC+6", "Omsk"},
    {7 * 2, "UTC+7", "Bangkok"},
    {8 * 2, "UTC+8", "Beijing"},
    {9 * 2, "UTC+9", "Tokyo"},
    {10 * 2, "UTC+10", "Sydney"}};

int getCurrentUTCOffset()
{
  return utcOffsets[config.utcOffsetIdx].offset;
}

int getTotalUTCOffsets()
{
  return sizeof(utcOffsets) / sizeof(utcOffsets[0]);
}

String webUtcOffsetSelector()
{
  String result;
  for (int i = 0; i < getTotalUTCOffsets(); i++)
  {
    char line[64];
    sprintf(line,
            "<option value='%d'%s>%s (%s)</option>",
            i,
            (i == config.utcOffsetIdx) ? " selected" : "",
            utcOffsets[i].city,
            utcOffsets[i].desc);
    result += line;
  }
  return result;
}
