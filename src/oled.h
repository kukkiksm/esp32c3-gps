#pragma once
#include <Arduino.h>

#define SDA_PIN 5
#define SCL_PIN 6

void oledInit();
void oledPrint(String line1, String line2 = "", String line3 = "", String line4 = "");
