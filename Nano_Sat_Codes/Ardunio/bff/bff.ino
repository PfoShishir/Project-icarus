// BNO055 + BMP280 — Clean & Efficient
// SPDX-License-Identifier: MIT

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// ── Config ────────────────────────────────────────────────────────────────────
#define SERIAL_BAUD     115200
#define LOOP_DELAY_MS   1000
#define SEA_LEVEL_HPA   1013.25f   // ← update to your local forecast

// ── Sensor objects ────────────────────────────────────────────────────────────
Adafruit_BNO055 bno(55, 0x28, &Wire);
Adafruit_BMP280 bmp;

// ── BNO055 vector table ───────────────────────────────────────────────────────
struct VectorDef {
  Adafruit_BNO055::adafruit_vector_type_t type;
  const char* label;
};

static const VectorDef VECTORS[] = {
  { Adafruit_BNO055::VECTOR_EULER,        "Euler (deg)"   },
  { Adafruit_BNO055::VECTOR_GYROSCOPE,    "Gyro (rad/s)"  },
  { Adafruit_BNO055::VECTOR_LINEARACCEL,  "Lin Accel (m/s²)" },
  { Adafruit_BNO055::VECTOR_ACCELEROMETER,"Accel (m/s²)"  },
  { Adafruit_BNO055::VECTOR_MAGNETOMETER, "Mag (uT)"      },
  { Adafruit_BNO055::VECTOR_GRAVITY,      "Gravity (m/s²)"},
};
static const uint8_t NUM_VECTORS = sizeof(VECTORS) / sizeof(VECTORS[0]);

// ── Helpers ───────────────────────────────────────────────────────────────────
static void haltOnError(const __FlashStringHelper* msg) {
  Serial.println(msg);
  while (true) delay(10);
}

static void printXYZ(const char* label, const imu::Vector<3>& v) {
  Serial.print("  ");
  Serial.print(label);
  Serial.print(": x=");
  Serial.print(v.x(), 4);
  Serial.print("  y=");
  Serial.print(v.y(), 4);
  Serial.print("  z=");
  Serial.println(v.z(), 4);
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial) delay(10);

  Serial.println(F("\n=== BNO055 + BMP280 ==="));

  if (!bno.begin())
    haltOnError(F("BNO055 not found — check wiring / I2C addr"));

  if (!bmp.begin())
    haltOnError(F("BMP280 not found — check wiring / I2C addr"));

  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,    // temperature
    Adafruit_BMP280::SAMPLING_X16,   // pressure
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );

  Serial.println(F("Sensors ready.\n"));
  delay(1000);
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  // --- BNO055 ---
  Serial.println(F("── BNO055 ──"));
  for (uint8_t i = 0; i < NUM_VECTORS; i++) {
    imu::Vector<3> v = bno.getVector(VECTORS[i].type);
    printXYZ(VECTORS[i].label, v);
  }

  // Calibration status (0-3 per sensor; 3 = fully calibrated)
  uint8_t sys, gyro, accel, mag;
  bno.getCalibration(&sys, &gyro, &accel, &mag);
  Serial.print(F("  Cal  sys="));  Serial.print(sys);
  Serial.print(F("  gyro=")); Serial.print(gyro);
  Serial.print(F("  accel="));Serial.print(accel);
  Serial.print(F("  mag="));  Serial.println(mag);

  // --- BMP280 ---
  Serial.println(F("── BMP280 ──"));
  Serial.print(F("  Temp     : ")); Serial.print(bmp.readTemperature(), 2); Serial.println(F(" °C"));
  Serial.print(F("  Pressure : ")); Serial.print(bmp.readPressure() / 100.0f, 2); Serial.println(F(" hPa"));
  Serial.print(F("  Altitude : ")); Serial.print(bmp.readAltitude(SEA_LEVEL_HPA), 2); Serial.println(F(" m"));
  Serial.println();

  delay(LOOP_DELAY_MS);
}
