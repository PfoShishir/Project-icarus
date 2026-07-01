#include <Wire.h>

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.println(F("Scanning I2C bus..."));

  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print(F("Device found at address 0x"));
      if (addr < 16) Serial.print(F("0"));
      Serial.println(addr, HEX);
    }
  }
  Serial.println(F("Scan complete."));
}

void loop() {}