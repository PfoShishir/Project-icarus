//reciever bff
#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS  16
#define RFM95_RST 17
#define RFM95_INT 21
#define RF95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

struct TelemetryPacket {
  float euler_x, euler_y, euler_z;
  float gyro_x, gyro_y, gyro_z;
  float laccel_x, laccel_y, laccel_z;
  float temperature;
  float pressure;
  float altitude;
  uint8_t cal_sys, cal_gyro, cal_accel, cal_mag;
};

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, LOW); delay(10);
  digitalWrite(RFM95_RST, HIGH); delay(10);

  if (!rf95.init()) { Serial.println("RX init FAILED"); while (1); }
  rf95.setFrequency(RF95_FREQ);
  Serial.println("RX ready — listening...");
}

void loop() {
  if (rf95.available()) {
    TelemetryPacket pkt;
    uint8_t len = sizeof(pkt);

    if (rf95.recv((uint8_t*)&pkt, &len)) {
      Serial.println("──── TELEMETRY ────");
      Serial.print("Euler    : x="); Serial.print(pkt.euler_x, 2);
      Serial.print("  y=");          Serial.print(pkt.euler_y, 2);
      Serial.print("  z=");          Serial.println(pkt.euler_z, 2);

      Serial.print("Gyro     : x="); Serial.print(pkt.gyro_x, 4);
      Serial.print("  y=");          Serial.print(pkt.gyro_y, 4);
      Serial.print("  z=");          Serial.println(pkt.gyro_z, 4);

      Serial.print("Lin Accel: x="); Serial.print(pkt.laccel_x, 4);
      Serial.print("  y=");          Serial.print(pkt.laccel_y, 4);
      Serial.print("  z=");          Serial.println(pkt.laccel_z, 4);

      Serial.print("Temp     : "); Serial.print(pkt.temperature, 2); Serial.println(" C");
      Serial.print("Pressure : "); Serial.print(pkt.pressure, 2);    Serial.println(" hPa");
      Serial.print("Altitude : "); Serial.print(pkt.altitude, 2);    Serial.println(" m");

      Serial.print("Cal  sys=");   Serial.print(pkt.cal_sys);
      Serial.print("  gyro=");     Serial.print(pkt.cal_gyro);
      Serial.print("  accel=");    Serial.print(pkt.cal_accel);
      Serial.print("  mag=");      Serial.println(pkt.cal_mag);

      Serial.print("RSSI: "); Serial.println(rf95.lastRssi());
      Serial.println("───────────────────");
    }
  }
}