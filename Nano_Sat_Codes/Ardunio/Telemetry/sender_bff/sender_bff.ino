//sender bff
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <RH_RF95.h>

#define RFM95_CS  4
#define RFM95_RST 2
#define RFM95_INT 3
#define RF95_FREQ 915.0
#define SEA_LEVEL_HPA 1013.25f

RH_RF95 rf95(RFM95_CS, RFM95_INT);
Adafruit_BNO055 bno(55, 0x28, &Wire);
Adafruit_BMP280 bmp;

struct TelemetryPacket {
  // Euler
  float euler_x, euler_y, euler_z;
  // Gyro
  float gyro_x, gyro_y, gyro_z;
  // Linear Accel
  float laccel_x, laccel_y, laccel_z;
  // BMP280
  float temperature;
  float pressure;
  float altitude;
  // Calibration
  uint8_t cal_sys, cal_gyro, cal_accel, cal_mag;
};

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, LOW); delay(10);
  digitalWrite(RFM95_RST, HIGH); delay(10);

  if (!rf95.init()) { Serial.println("RF95 init FAILED"); while (1); }
  rf95.setFrequency(RF95_FREQ);
  rf95.setTxPower(13, false);

  if (!bno.begin()) { Serial.println("BNO055 not found"); while (1); }
  if (!bmp.begin()) { Serial.println("BMP280 not found"); while (1); }

  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,
    Adafruit_BMP280::SAMPLING_X16,
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );

  Serial.println("TX ready — sending telemetry...");
}

void loop() {
  TelemetryPacket pkt;

  imu::Vector<3> euler  = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  imu::Vector<3> gyro   = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  imu::Vector<3> laccel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

  pkt.euler_x = euler.x();  pkt.euler_y = euler.y();  pkt.euler_z = euler.z();
  pkt.gyro_x  = gyro.x();   pkt.gyro_y  = gyro.y();   pkt.gyro_z  = gyro.z();
  pkt.laccel_x = laccel.x(); pkt.laccel_y = laccel.y(); pkt.laccel_z = laccel.z();

  pkt.temperature = bmp.readTemperature();
  pkt.pressure    = bmp.readPressure() / 100.0f;
  pkt.altitude    = bmp.readAltitude(SEA_LEVEL_HPA);

  bno.getCalibration(&pkt.cal_sys, &pkt.cal_gyro, &pkt.cal_accel, &pkt.cal_mag);

  rf95.send((uint8_t*)&pkt, sizeof(pkt));
  rf95.waitPacketSent();

  Serial.println("Packet sent!");
  delay(1000);
}
