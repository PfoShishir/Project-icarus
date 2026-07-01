// gps BMP280 rtc and sd logger but log in everything new filed
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <Adafruit_BMP280.h>

SoftwareSerial gpsSerial(8, 7);
TinyGPSPlus gps;
RTC_PCF8523 rtc;
Adafruit_BMP280 bmp;
const int chipSelect = 10;
const int UTC_OFFSET = -4;  // EDT summer, change to -5 for EST winter
char filename[] = "test00.csv";

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  Wire.begin();

  Serial.println(F("GPS + RTC + BMP280 + SD Starting..."));

  // RTC init
  if (!rtc.begin()) {
    Serial.println(F("RTC NOT found!"));
    while (1);
  }
  Serial.println(F("RTC found"));

  // Comment out after first upload
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // BMP280 init
  if (!bmp.begin(0x77)) {
    Serial.println(F("BMP280 NOT found!"));
    while (1);
  }
  Serial.println(F("BMP280 found"));

  // BMP280 settings
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);

  // SD init
  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD init failed!"));
    while (1);
  }
  Serial.println(F("SD ready"));

  // Auto number file test00 to test99
  for (int i = 0; i < 100; i++) {
    filename[4] = '0' + (i / 10);
    filename[5] = '0' + (i % 10);
    if (!SD.exists(filename)) break;
  }
  Serial.print(F("Logging to: "));
  Serial.println(filename);

  // Write CSV header
  File f = SD.open(filename, FILE_WRITE);
  if (f) {
    f.println(F("date,local_time,gps_utc,rtc_synced,latitude,longitude,satellites,speed_kmh,temp_c,pressure_pa,altitude_m"));
    f.close();
  }

  Serial.println(F("All systems ready!"));
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

    // Sync RTC from GPS once
    if (!synced && gps.date.isValid() && gps.date.year() > 2020) {
      rtc.adjust(DateTime(
        gps.date.year(), gps.date.month(), gps.date.day(),
        gps.time.hour(), gps.time.minute(), gps.time.second()
      ));
      synced = true;
      Serial.println(F("RTC synced from GPS!"));
      now = rtc.now();
    }

    // Read BMP280
    float tempC    = bmp.readTemperature();
    float pressPA  = bmp.readPressure();
    float altM     = bmp.readAltitude(1013.25);

    // Calculate local time
    int localHour = now.hour() + UTC_OFFSET;
    if (localHour < 0)   localHour += 24;
    if (localHour >= 24) localHour -= 24;

    // Build buffers
    char dateBuf[12], localBuf[10], gpsBuf[10];

    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d",
      now.year(), now.month(), now.day());

    snprintf(localBuf, sizeof(localBuf), "%02d:%02d:%02d",
      localHour, now.minute(), now.second());

    if (gps.time.isValid()) {
      snprintf(gpsBuf, sizeof(gpsBuf), "%02d:%02d:%02d",
        gps.time.hour(), gps.time.minute(), gps.time.second());
    } else {
      snprintf(gpsBuf, sizeof(gpsBuf), "--:--:--");
    }

    // Print to Serial
    Serial.println(F("----------"));
    Serial.print(F("Date:         ")); Serial.println(dateBuf);
    Serial.print(F("Local Time:   ")); Serial.println(localBuf);
    Serial.print(F("GPS UTC:      ")); Serial.println(gpsBuf);
    Serial.print(F("Temperature:  ")); Serial.print(tempC);   Serial.println(F(" C"));
    Serial.print(F("Pressure:     ")); Serial.print(pressPA); Serial.println(F(" Pa"));
    Serial.print(F("Altitude:     ")); Serial.print(altM);    Serial.println(F(" m"));

    if (gps.location.isValid()) {
      Serial.print(F("Latitude:     ")); Serial.println(gps.location.lat(), 6);
      Serial.print(F("Longitude:    ")); Serial.println(gps.location.lng(), 6);
      Serial.print(F("Satellites:   ")); Serial.println(gps.satellites.value());
      Serial.print(F("Speed km/h:   ")); Serial.println(gps.speed.kmph());
    } else {
      Serial.println(F("GPS: No fix yet..."));
    }

    // Log to SD
    File dataFile = SD.open(filename, FILE_WRITE);
    if (dataFile) {
      dataFile.print(dateBuf);                    dataFile.print(F(","));
      dataFile.print(localBuf);                   dataFile.print(F(","));
      dataFile.print(gpsBuf);                     dataFile.print(F(","));
      dataFile.print(synced ? F("1") : F("0"));   dataFile.print(F(","));

      if (gps.location.isValid()) {
        dataFile.print(gps.location.lat(), 6);    dataFile.print(F(","));
        dataFile.print(gps.location.lng(), 6);    dataFile.print(F(","));
        dataFile.print(gps.satellites.value());   dataFile.print(F(","));
        dataFile.print(gps.speed.kmph());         dataFile.print(F(","));
      } else {
        dataFile.print(F(",,,,"));
      }

      dataFile.print(tempC);                      dataFile.print(F(","));
      dataFile.print(pressPA);                    dataFile.print(F(","));
      dataFile.println(altM);
      dataFile.close();
      Serial.println(F("Logged"));
    } else {
      Serial.println(F("ERROR: SD open failed"));
    }
  }
}