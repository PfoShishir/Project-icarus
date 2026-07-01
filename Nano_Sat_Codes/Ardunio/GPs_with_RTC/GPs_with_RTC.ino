#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include "RTClib.h"

SoftwareSerial gpsSerial(8, 7);
TinyGPSPlus gps;
RTC_PCF8523 rtc;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  Wire.begin();

  Serial.println("GPS + RTC Starting...");

  if (!rtc.begin()) {
    Serial.println("RTC NOT found!");
    while (1);
  }
  Serial.println("RTC found");

  // Comment out after first upload
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  // Feed GPS
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  // Always print RTC time every second
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();

    DateTime now = rtc.now();

    Serial.println("----------");

    // RTC time (works indoors always)
    Serial.print("RTC Time: ");
    Serial.print(now.year()); Serial.print("-");
    if (now.month() < 10) Serial.print("0");
    Serial.print(now.month()); Serial.print("-");
    if (now.day() < 10) Serial.print("0");
    Serial.print(now.day()); Serial.print(" ");
    if (now.hour() < 10) Serial.print("0");
    Serial.print(now.hour()); Serial.print(":");
    if (now.minute() < 10) Serial.print("0");
    Serial.print(now.minute()); Serial.print(":");
    if (now.second() < 10) Serial.print("0");
    Serial.println(now.second());

    // GPS data (only when fix available)
    if (gps.location.isValid()) {
      Serial.print("Latitude:  ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("Longitude: ");
      Serial.println(gps.location.lng(), 6);
      Serial.print("Satellites: ");
      Serial.println(gps.satellites.value());
      Serial.print("Speed (km/h): ");
      Serial.println(gps.speed.kmph());

      // Sync RTC from GPS once
      static bool synced = false;
      if (!synced && gps.date.year() > 2020) {
        rtc.adjust(DateTime(
          gps.date.year(), gps.date.month(), gps.date.day(),
          gps.time.hour(), gps.time.minute(), gps.time.second()
        ));
        synced = true;
        Serial.println("RTC synced from GPS!");
      }

    } else {
      Serial.println("GPS: No fix yet...");
    }
  }
}
