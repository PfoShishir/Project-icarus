#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_AS7343.h>

#define SD_CS 10

Adafruit_AS7343 as7343;

char filename[] = "SPEC62.CSV";

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("AS7343 + SD Logger Starting..."));

  // AS7343 initialization
  if (!as7343.begin()) {
    Serial.println(F("AS7343 not found!"));
    while (1);
  }

  Serial.println(F("AS7343 found"));

  as7343.setGain(AS7343_GAIN_64X);
  as7343.setATIME(29);
  as7343.setASTEP(599);

  // SD initialization
  pinMode(SD_CS, OUTPUT);

  if (!SD.begin(SD_CS)) {
    Serial.println(F("SD init failed!"));
    while (1);
  }

  Serial.println(F("SD ready"));

  // Create file and header if needed
  File file = SD.open(filename, FILE_WRITE);

  if (file) {

    if (file.size() == 0) {
      file.println(
        F("F1_405nm,F2_425nm,FZ_450nm,F3_475nm,"
          "F4_515nm,F5_550nm,FY_555nm,FXL_600nm,"
          "F6_640nm,F7_690nm,F8_745nm,NIR_855nm,VIS")
      );
    }

    file.close();

  } else {
    Serial.println(F("Failed to open SPEC62.CSV"));
    while (1);
  }

  Serial.println(F("Logging started"));
}

void loop() {

  uint16_t r[18];

  if (!as7343.readAllChannels(r)) {
    Serial.println(F("AS7343 read failed"));
    delay(1000);
    return;
  }

  // Print CSV line to Serial Monitor
  Serial.print(r[AS7343_CHANNEL_F1]);  Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_F2]);  Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_FZ]);  Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_F3]);  Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_F4]);  Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_F5]);  Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_FY]);  Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_FXL]); Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_F6]);  Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_F7]);  Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_F8]);  Serial.print(",");
  Serial.print(r[AS7343_CHANNEL_NIR]); Serial.print(",");
  Serial.println(r[AS7343_CHANNEL_VIS_TL_0]);

  // Save to SD card
  File file = SD.open(filename, FILE_WRITE);

  if (file) {

    file.print(r[AS7343_CHANNEL_F1]);  file.print(",");
    file.print(r[AS7343_CHANNEL_F2]);  file.print(",");
    file.print(r[AS7343_CHANNEL_FZ]);  file.print(",");
    file.print(r[AS7343_CHANNEL_F3]);  file.print(",");
    file.print(r[AS7343_CHANNEL_F4]);  file.print(",");
    file.print(r[AS7343_CHANNEL_F5]);  file.print(",");
    file.print(r[AS7343_CHANNEL_FY]);  file.print(",");
    file.print(r[AS7343_CHANNEL_FXL]); file.print(",");
    file.print(r[AS7343_CHANNEL_F6]);  file.print(",");
    file.print(r[AS7343_CHANNEL_F7]);  file.print(",");
    file.print(r[AS7343_CHANNEL_F8]);  file.print(",");
    file.print(r[AS7343_CHANNEL_NIR]); file.print(",");
    file.println(r[AS7343_CHANNEL_VIS_TL_0]);

    file.close();

    Serial.println(F("Logged"));

  } else {
    Serial.println(F("ERROR: SD open failed"));
  }

  delay(20000);
}