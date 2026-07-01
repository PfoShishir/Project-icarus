#include <Wire.h>
#include "Adafruit_MCP9808.h"
#include "RTClib.h"

Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
RTC_DS1307 rtc;

void setup() {
   
  Serial.begin(9600);
  delay(2000);
  
  if (!tempsensor.begin()) {
    Serial.println("MCP9808 not found!");
    while (1);
  }

  if (!rtc.begin()) {
    Serial.println("RTC not found!");
    while (1);
  }

  rtc.adjust(DateTime(2026, 5, 22, 8, 8, 0));
  Serial.println("Recording Temperature");
  
    pinMode(12,OUTPUT);

  
  
}

void loop() {
  DateTime now = rtc.now();
  float tempC = tempsensor.readTempC();
  float tempF = tempC * 9.0 / 5.0 + 32;
  float tempK = tempC + 273.15;

  Serial.print(now.year());   Serial.print("/");
  Serial.print(now.month());  Serial.print("/");
  Serial.print(now.day());    Serial.print(" ");
  Serial.print(now.hour());   Serial.print(":");
  Serial.print(now.minute()); Serial.print(":");
  Serial.print(now.second()); Serial.print(" | ");

  Serial.print(tempC); Serial.print(" C | ");
  Serial.print(tempF); Serial.print(" F | ");
  Serial.print(tempK); Serial.println(" K");
  
    digitalWrite(12, HIGH);
    delay(500);
    digitalWrite(12,LOW);

    delay(1000);
  

}
