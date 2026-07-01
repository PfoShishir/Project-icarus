#include <SPI.h>

// Add or remove CS pins you want to scan
byte csPins[] = {4, 7, 8, 9, 10};

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  SPI.begin();
  Serial.println("Scanning SPI devices...");
  Serial.println("------------------------");

  for (int i = 0; i < sizeof(csPins); i++) {
    pinMode(csPins[i], OUTPUT);
    digitalWrite(csPins[i], HIGH);
  }

  for (int i = 0; i < sizeof(csPins); i++) {
    byte csPin = csPins[i];

    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(csPin, LOW);
    SPI.transfer(0x42);
    byte val = SPI.transfer(0x00);
    digitalWrite(csPin, HIGH);
    SPI.endTransaction();

    Serial.print("CS pin ");
    Serial.print(csPin);
    Serial.print(": 0x");
    Serial.print(val, HEX);

    if (val == 0x12) {
      Serial.println("  <-- RFM95W detected!");
    } else if (val == 0xFF || val == 0x00) {
      Serial.println("  <-- nothing");
    } else {
      Serial.println("  <-- unknown device");
    }
  }

  Serial.println("------------------------");
  Serial.println("Done.");
}

void loop() {}