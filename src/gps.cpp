#include "gps.h"
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

static TinyGPSPlus gps;
static HardwareSerial gpsSerial(1);

void gpsInit(int rxPin, int txPin)
{
  gpsSerial.begin(9600, SERIAL_8N1, rxPin, txPin);
}

bool gpsRead(gps_data_t &data)
{
  while (gpsSerial.available())
  {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated())
  {
    data.lat = gps.location.lat(); 
    data.lon = gps.location.lng();  
    data.alt = gps.altitude.meters();
    data.sats = gps.satellites.value();
    data.hh = gps.time.hour();
    data.mm = gps.time.minute();
    data.ss = gps.time.second();
    data.day = gps.date.day();
    data.month = gps.date.month();
    data.year = gps.date.year();
    return true;
  }

  return false;
}
