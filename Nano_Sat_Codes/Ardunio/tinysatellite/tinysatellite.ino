#include <SoftwareSerial.h>
#include <TinyGPS++.h>

SoftwareSerial gpsSerial(8, 7);
TinyGPSPlus gps;

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);

  Serial.println("GPS Test Starting...");
}

void loop() {

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated()) {

    Serial.println("----------");

    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);

    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);

    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());

    Serial.print("Speed (km/h): ");
    Serial.println(gps.speed.kmph());

    Serial.print("Date: ");
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.print(gps.date.day());
    Serial.print("/");
    Serial.println(gps.date.year());

    Serial.print("Time (UTC): ");
    Serial.print(gps.time.hour());
    Serial.print(":");
    Serial.print(gps.time.minute());
    Serial.print(":");
    Serial.println(gps.time.second());
  }
}