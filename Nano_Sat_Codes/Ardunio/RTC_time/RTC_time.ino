#include <Wire.h>
#include "RTClib.h"

RTC_PCF8523 rtc;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  Serial.println("RTC PCF8523 Test Starting...");

  if (!rtc.begin()) {
    Serial.println("RTC NOT found");
    while (1);
  }

  Serial.println("RTC found");

  // Set time ONLY ONCE (then comment this line after upload)
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.println("RTC time set");
}

void loop() {
  DateTime now = rtc.now();

  Serial.print("Time: ");
  Serial.print(now.year()); Serial.print("-");
  Serial.print(now.month()); Serial.print("-");
  Serial.print(now.day()); Serial.print(" ");
  Serial.print(now.hour()); Serial.print(":");
  Serial.print(now.minute()); Serial.print(":");
  Serial.println(now.second());

  delay(1000);
}