#include <Wire.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  Serial.println("BNO055 Test Starting...");

  if (!bno.begin()) {
    Serial.println("BNO055 NOT detected. Check wiring!");
    while (1);
  }

  delay(1000);
  bno.setExtCrystalUse(true);
}

void loop() {
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  Serial.print("Heading: ");
  Serial.print(euler.x());
  Serial.print("  Roll: ");
  Serial.print(euler.y());
  Serial.print("  Pitch: ");
  Serial.println(euler.z());

  delay(500);
}