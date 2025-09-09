#pragma once

#include <Arduino.h>

void initLedSignal(uint8_t pin);
void updateLedSignal();
void setLedOffline();
void setLedOnline();

void updateMorseMessage(float lat, float lon);  
void toggleLedMorseMode();
void updateLedMorse();
