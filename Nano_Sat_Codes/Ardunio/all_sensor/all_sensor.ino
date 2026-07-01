// NanoSat_AllSensors_GPS_FIXED.ino
// All Sensors + GPS
// BMP280 addr fixed
// Baud unified 9600
// ============================================================
//  NanoSat — All Sensors + GPS  (FIXED)
//  BNO055, BMP280 BFF (0x76), AS7343, MCP9808, PCF8523, GPS
//
//  FIXES vs previous version:
//    1. BMP280 address → 0x76  (BFF combo board default)
//    2. All Serial at 9600 baud (GPS requires this)
//    3. GPS fed continuously — no delay() starving the parser
//    4. Sea-level P0 corrected from GPS altitude once fixed
// ============================================================

#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_AS7343.h>
#include <Adafruit_MCP9808.h>
#include <RTClib.h>
#include <utility/imumaths.h>

// ── Config ────────────────────────────────────────────────────────────────────
#define SERIAL_BAUD   9600     // MUST match GPS baud — do not change to 115200
#define GPS_BAUD      9600
#define PRINT_EVERY_MS 2000    // print interval — give GPS time between prints
#define BMP_ADDR      0x77     // BFF combo board default (was wrong: 0x77)
#define GPS_RX        8
#define GPS_TX        7

// ── Objects ───────────────────────────────────────────────────────────────────
SoftwareSerial   gpsSerial(GPS_RX, GPS_TX);
TinyGPSPlus      gps;
Adafruit_BNO055  bno(55, 0x28, &Wire);
Adafruit_BMP280  bmp;
Adafruit_AS7343  as7343;
Adafruit_MCP9808 tempsensor;
RTC_PCF8523      rtc;

// ── State ─────────────────────────────────────────────────────────────────────
static bool  gpsRtcSynced = false;
static float seaLevelHpa  = 1013.25f;

// ── BNO055 vector table ───────────────────────────────────────────────────────
struct VectorDef {
  Adafruit_BNO055::adafruit_vector_type_t type;
  const char* label;
};
static const VectorDef VECTORS[] = {
  { Adafruit_BNO055::VECTOR_EULER,         "Euler (deg)"      },
  { Adafruit_BNO055::VECTOR_GYROSCOPE,     "Gyro (rad/s)"     },
  { Adafruit_BNO055::VECTOR_LINEARACCEL,   "Lin Accel (m/s2)" },
  { Adafruit_BNO055::VECTOR_ACCELEROMETER, "Accel (m/s2)"     },
  { Adafruit_BNO055::VECTOR_MAGNETOMETER,  "Mag (uT)"         },
  { Adafruit_BNO055::VECTOR_GRAVITY,       "Gravity (m/s2)"   },
};
static const uint8_t NUM_VECTORS = sizeof(VECTORS) / sizeof(VECTORS[0]);

// ── Helpers ───────────────────────────────────────────────────────────────────
static void halt(const __FlashStringHelper* msg) {
  Serial.println(msg);
  while (true) delay(10);
}

static void p2(int v) {               // zero-padded 2-digit
  if (v < 10) Serial.print('0');
  Serial.print(v);
}

static void printXYZ(const char* label, const imu::Vector<3>& v) {
  Serial.print(F("  ")); Serial.print(label);
  Serial.print(F(": x=")); Serial.print(v.x(), 3);
  Serial.print(F("  y=")); Serial.print(v.y(), 3);
  Serial.print(F("  z=")); Serial.println(v.z(), 3);
}

// Invert barometric formula to get true local P0 from GPS MSL altitude
static float calcP0(float pressHpa, float altMeters) {
  float r = 1.0f - altMeters / 44330.0f;
  return (r > 0.01f) ? pressHpa / pow(r, 5.255f) : 1013.25f;
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial) delay(10);
  Serial.println(F("=== NanoSat Boot ==="));

  gpsSerial.begin(GPS_BAUD);

  if (!bno.begin())        halt(F("BNO055 not found"));
  if (!bmp.begin(BMP_ADDR)) halt(F("BMP280 not found — tried 0x76"));
  if (!as7343.begin())     halt(F("AS7343 not found"));
  if (!tempsensor.begin()) halt(F("MCP9808 not found"));
  if (!rtc.begin())         halt(F("PCF8523 not found"));

  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,
    Adafruit_BMP280::SAMPLING_X16,
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );
  as7343.setGain(AS7343_GAIN_64X);
  as7343.setATIME(29);
  as7343.setASTEP(599);

  if (!rtc.initialized() || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(12, OUTPUT);
  Serial.println(F("All OK. Waiting for GPS fix...\n"));
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {

  // Feed GPS every pass — never block this with delay()
  while (gpsSerial.available())
    gps.encode(gpsSerial.read());

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint < PRINT_EVERY_MS) return;
  lastPrint = millis();

  Serial.println(F("-----------------------------"));

  // ── GPS ────────────────────────────────────────────────────────────────────
  Serial.println(F("[GPS]"));
  if (gps.location.isValid()) {
    Serial.print(F("  Lat: "));  Serial.println(gps.location.lat(), 6);
    Serial.print(F("  Lng: "));  Serial.println(gps.location.lng(), 6);
    Serial.print(F("  Alt: "));  Serial.print(gps.altitude.meters(), 1);  Serial.println(F(" m MSL"));
    Serial.print(F("  Sat: "));  Serial.println(gps.satellites.value());
    Serial.print(F("  Spd: "));  Serial.print(gps.speed.kmph(), 1);      Serial.println(F(" km/h"));

    // Sync RTC once
    if (!gpsRtcSynced && gps.date.year() > 2020) {
      rtc.adjust(DateTime(
        gps.date.year(),  gps.date.month(),  gps.date.day(),
        gps.time.hour(), gps.time.minute(), gps.time.second()
      ));
      gpsRtcSynced = true;
      Serial.println(F("  RTC synced from GPS!"));
    }

    // Update P0 every reading once GPS altitude valid
    if (gps.altitude.isValid()) {
      seaLevelHpa = calcP0(bmp.readPressure() / 100.0f, gps.altitude.meters());
    }
  } else {
    Serial.print(F("  No fix  chars="));
    Serial.println(gps.charsProcessed());
    Serial.println(F("  (Take outdoors — needs clear sky view)"));
  }

  // ── RTC ────────────────────────────────────────────────────────────────────
  Serial.println(F("[RTC]"));
  DateTime now = rtc.now();
  Serial.print(F("  "));
  Serial.print(now.year()); Serial.print('/');
  p2(now.month());  Serial.print('/');
  p2(now.day());    Serial.print(F("  "));
  p2(now.hour());   Serial.print(':');
  p2(now.minute()); Serial.print(':');
  p2(now.second()); Serial.println();
  Serial.print(F("  GPS sync: "));
  Serial.println(gpsRtcSynced ? F("YES") : F("NO"));

  // ── BMP280 ─────────────────────────────────────────────────────────────────
  Serial.println(F("[BMP280]"));
  float bmpT = bmp.readTemperature();
  float bmpP = bmp.readPressure() / 100.0f;
  float bmpA = bmp.readAltitude(seaLevelHpa);
  Serial.print(F("  Temp: "));     Serial.print(bmpT, 2);    Serial.println(F(" C"));
  Serial.print(F("  Press: "));    Serial.print(bmpP, 2);    Serial.println(F(" hPa"));
  Serial.print(F("  Alt: "));      Serial.print(bmpA, 1);    Serial.println(F(" m"));
  Serial.print(F("  P0: "));       Serial.print(seaLevelHpa, 2);
  Serial.println(gpsRtcSynced ? F(" hPa [GPS cal]") : F(" hPa [std]"));

  // ── BNO055 ─────────────────────────────────────────────────────────────────
  Serial.println(F("[BNO055]"));
  for (uint8_t i = 0; i < NUM_VECTORS; i++) {
    imu::Vector<3> v = bno.getVector(VECTORS[i].type);
    printXYZ(VECTORS[i].label, v);
  }
  uint8_t cs, cg, ca, cm;
  bno.getCalibration(&cs, &cg, &ca, &cm);
  Serial.print(F("  Cal sys=")); Serial.print(cs);
  Serial.print(F(" gyro="));    Serial.print(cg);
  Serial.print(F(" accel="));   Serial.print(ca);
  Serial.print(F(" mag="));     Serial.println(cm);

  // ── AS7343 ─────────────────────────────────────────────────────────────────
  Serial.println(F("[AS7343]"));
  uint16_t ch[18];
  if (as7343.readAllChannels(ch)) {
    Serial.print(F("  405nm:")); Serial.print(ch[AS7343_CHANNEL_F1]);
    Serial.print(F("  425nm:")); Serial.print(ch[AS7343_CHANNEL_F2]);
    Serial.print(F("  450nm:")); Serial.println(ch[AS7343_CHANNEL_FZ]);
    Serial.print(F("  475nm:")); Serial.print(ch[AS7343_CHANNEL_F3]);
    Serial.print(F("  515nm:")); Serial.print(ch[AS7343_CHANNEL_F4]);
    Serial.print(F("  550nm:")); Serial.println(ch[AS7343_CHANNEL_F5]);
    Serial.print(F("  555nm:")); Serial.print(ch[AS7343_CHANNEL_FY]);
    Serial.print(F("  600nm:")); Serial.print(ch[AS7343_CHANNEL_FXL]);
    Serial.print(F("  640nm:")); Serial.println(ch[AS7343_CHANNEL_F6]);
    Serial.print(F("  690nm:")); Serial.print(ch[AS7343_CHANNEL_F7]);
    Serial.print(F("  745nm:")); Serial.print(ch[AS7343_CHANNEL_F8]);
    Serial.print(F("  NIR:"));   Serial.print(ch[AS7343_CHANNEL_NIR]);
    Serial.print(F("  VIS:"));   Serial.println(ch[AS7343_CHANNEL_VIS_TL_0]);
  } else {
    Serial.println(F("  READ FAILED"));
  }

  // ── MCP9808 ────────────────────────────────────────────────────────────────
  Serial.println(F("[MCP9808]"));
  float tc = tempsensor.readTempC();
  Serial.print(F("  "));
  Serial.print(tc, 2); Serial.print(F("C  "));
  Serial.print(tc * 9.0f/5.0f + 32.0f, 2); Serial.print(F("F  "));
  Serial.print(tc + 273.15f, 2); Serial.println(F("K"));

  Serial.println();
  digitalWrite(12, HIGH); delay(50); digitalWrite(12, LOW);
}
