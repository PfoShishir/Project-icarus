#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"

const int SDpin        = 10;
const int nInterrupt   = 0;
const int interruptPin = 2;
const int UTC_OFFSET   = -4;

volatile unsigned long knt       = 0;
volatile unsigned long M         = 0;
volatile bool          do_log    = false;
volatile unsigned long saved_knt = 0;

unsigned long millis_start;

RTC_PCF8523 rtc;
File logfile;
char filename[16];

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println(F("ERROR: RTC not found."));
    while (true);
  }
  if (!rtc.initialized() || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.start();

  if (!SD.begin(SDpin)) {
    Serial.println(F("ERROR: SD card failed."));
    while (true);
  }

  // Auto-increment filename: GEIGER01.CSV, GEIGER02.CSV ...
  for (int i = 1; i <= 99; i++) {
    snprintf(filename, sizeof(filename), "GEIGER%02d.CSV", i);
    if (!SD.exists(filename)) break;
  }

  logfile = SD.open(filename, FILE_WRITE);
  if (!logfile) {
    Serial.println(F("ERROR: Could not create file."));
    while (true);
  }

  // Header written once to fresh file
  logfile.println(F("date,time,cpm,uSv_hr"));
  logfile.flush();

  Serial.print(F("Logging to: "));
  Serial.println(filename);
  Serial.println(F("date,time,cpm,uSv_hr"));
  Serial.println(F("Logging started."));

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(nInterrupt, isr, FALLING);
  millis_start = millis();
}

void loop() {
  if (do_log) {
    do_log = false;

    DateTime now = rtc.now();

    int localHour = now.hour() + UTC_OFFSET;
    if (localHour < 0)   localHour += 24;
    if (localHour >= 24) localHour -= 24;

    char dateBuf[12], timeBuf[10];
    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d",
             now.year(), now.month(), now.day());
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d",
             localHour, now.minute(), now.second());

    float uSv = saved_knt / 151.0;

    // SD
    logfile.print(dateBuf);   logfile.print(F(","));
    logfile.print(timeBuf);   logfile.print(F(","));
    logfile.print(saved_knt); logfile.print(F(","));
    logfile.println(uSv, 3);
    logfile.flush();

    // Serial
    Serial.print(dateBuf);   Serial.print(F(","));
    Serial.print(timeBuf);   Serial.print(F(","));
    Serial.print(saved_knt); Serial.print(F(","));
    Serial.println(uSv, 3);
  }
}

void isr() {
  knt++;
  M = millis();
  if ((M - millis_start) > 60000UL) {
    saved_knt    = knt;
    do_log       = true;
    knt          = 0;
    millis_start = M;
  }
}