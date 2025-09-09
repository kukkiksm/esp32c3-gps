#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) gps_data_t {
  float lat;
  float lon;
  float alt;
  uint8_t sats;
  uint8_t hh;
  uint8_t mm;
  uint8_t ss;
  uint8_t day;
  uint8_t month;
  uint16_t year;
} gps_data_t;

void gpsInit(int rxPin, int txPin);
bool gpsRead(gps_data_t &data);
