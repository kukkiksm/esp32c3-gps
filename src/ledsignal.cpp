#include "ledsignal.h"

static uint8_t ledPin = 8;
static bool ledState = false;
static unsigned long lastToggle = 0;
static unsigned long onDuration = 2000;
static unsigned long offDuration = 1000;
static bool enabled = false;

void initLedSignal(uint8_t pin)
{
  ledPin = pin;
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); 
}

void setLedOffline()
{
  enabled = true;
  onDuration = 2000;
  offDuration = 1000;
}

void setLedOnline()
{
  enabled = false;
  ledState = false;
  lastToggle = millis();
  digitalWrite(ledPin, HIGH);  
}

void updateLedSignal()
{
  if (!enabled) return;

  unsigned long now = millis();
  if (ledState && now - lastToggle >= onDuration)
  {
    ledState = false;
    lastToggle = now;
    digitalWrite(ledPin, HIGH);  
  }
  else if (!ledState && now - lastToggle >= offDuration)
  {
    ledState = true;
    lastToggle = now;
    digitalWrite(ledPin, LOW);  
  }
}

static bool morseMode = false;
static String morseMessage = "";
static unsigned long lastMorseToggle = 0;
static int morseIndex = 0;
static bool morseOn = false;

void updateMorseMessage(float lat, float lon)
{
  morseMessage = "LAT:" + String(lat, 2) + " LON:" + String(lon, 2);
}

void toggleLedMorseMode()
{
  morseMode = !morseMode;
  if (morseMode)
  {
    morseIndex = 0;
    lastMorseToggle = millis();
    morseOn = false;
    digitalWrite(ledPin, HIGH); 
  }
  else
  {
    digitalWrite(ledPin, HIGH); 
  }
}

void updateLedMorse()
{
  if (!morseMode || morseMessage.length() == 0) return;

  unsigned long now = millis();
  char c = morseMessage.charAt(morseIndex);

  if (morseOn)
  {
    if (now - lastMorseToggle >= 200)
    {
      digitalWrite(ledPin, HIGH);  
      morseOn = false;
      lastMorseToggle = now;

      morseIndex++;
      if (morseIndex >= morseMessage.length())
        morseIndex = 0;
    }
  }
  else
  {
    if (now - lastMorseToggle >= 200)
    {
      if (c != ' ')
        digitalWrite(ledPin, LOW);  
      else
        digitalWrite(ledPin, HIGH); 

      morseOn = true;
      lastMorseToggle = now;
    }
  }
}
