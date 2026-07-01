//gps_RTC with sd
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"

SoftwareSerial gpsSerial(8, 7);
TinyGPSPlus gps;
RTC_PCF8523 rtc;
const int chipSelect = 10;
const int UTC_OFFSET = -4;  // EDT summer, change to -5 for EST winter

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  Wire.begin();

  Serial.println(F("GPS + RTC + SD Starting..."));

  // RTC init
  if (!rtc.begin()) {
    Serial.println(F("RTC NOT found!"));
    while (1);
  }
  Serial.println(F("RTC found"));

  // Comment out after first upload
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // SD init
  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD init failed!"));
    while (1);
  }
  Serial.println(F("SD ready"));

  // Write CSV header if file doesnt exist
  if (!SD.exists("datalog.csv")) {
    File f = SD.open("datalog.csv", FILE_WRITE);
    if (f) {
      f.println(F("date,local_time,gps_utc_time,rtc_synced,latitude,longitude,satellites,speed_kmh"));
      f.close();
    }
  }
}

void loop() {
  // Feed GPS
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  static uint32_t lastLog = 0;
  static bool synced = false;

  if (millis() - lastLog >= 10000) {
    lastLog = millis();

    DateTime now = rtc.now();

    // Sync RTC from GPS once when fix available
    if (!synced && gps.date.isValid() && gps.date.year() > 2020) {
      // Store UTC in RTC
      rtc.adjust(DateTime(
        gps.date.year(), gps.date.month(), gps.date.day(),
        gps.time.hour(), gps.time.minute(), gps.time.second()
      ));
      synced = true;
      Serial.println(F("RTC synced from GPS!"));
      now = rtc.now(); // refresh after sync
    }

    // Calculate local time from RTC (UTC stored in RTC)
    int localHour = now.hour() + UTC_OFFSET;
    if (localHour < 0) localHour += 24;
    if (localHour >= 24) localHour -= 24;

    // Date buffer
    char dateBuf[12];
    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d",
      now.year(), now.month(), now.day());

    // Local time buffer (RTC + offset)
    char localBuf[10];
    snprintf(localBuf, sizeof(localBuf), "%02d:%02d:%02d",
      localHour, now.minute(), now.second());

    // GPS UTC time buffer
    char gpsBuf[10];
    if (gps.time.isValid()) {
      snprintf(gpsBuf, sizeof(gpsBuf), "%02d:%02d:%02d",
        gps.time.hour(), gps.time.minute(), gps.time.second());
    } else {
      snprintf(gpsBuf, sizeof(gpsBuf), "--:--:--");
    }

    // Print to Serial
    Serial.println(F("----------"));
    Serial.print(F("Date:          ")); Serial.println(dateBuf);
    Serial.print(F("Local Time:    ")); Serial.println(localBuf);
    Serial.print(F("GPS UTC Time:  ")); Serial.println(gpsBuf);

    if (gps.location.isValid()) {
      Serial.print(F("Latitude:      ")); Serial.println(gps.location.lat(), 6);
      Serial.print(F("Longitude:     ")); Serial.println(gps.location.lng(), 6);
      Serial.print(F("Satellites:    ")); Serial.println(gps.satellites.value());
      Serial.print(F("Speed km/h:    ")); Serial.println(gps.speed.kmph());
    } else {
      Serial.println(F("GPS: No fix yet..."));
    }

    // Log to SD
    File dataFile = SD.open("datalog.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.print(dateBuf);              dataFile.print(F(","));
      dataFile.print(localBuf);             dataFile.print(F(","));
      dataFile.print(gpsBuf);               dataFile.print(F(","));
      dataFile.print(synced ? F("1"):F("0")); dataFile.print(F(","));

      if (gps.location.isValid()) {
        dataFile.print(gps.location.lat(), 6); dataFile.print(F(","));
        dataFile.print(gps.location.lng(), 6); dataFile.print(F(","));
        dataFile.print(gps.satellites.value()); dataFile.print(F(","));
        dataFile.println(gps.speed.kmph());
      } else {
        dataFile.println(F(",,,,"));
      }
      dataFile.close();
      Serial.println(F("Logged"));
    } else {
      Serial.println(F("ERROR: SD open failed"));
    }
  }
}