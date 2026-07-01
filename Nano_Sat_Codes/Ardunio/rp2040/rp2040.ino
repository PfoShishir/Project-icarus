#include <SPI.h>
#include <RH_RF95.h>

// RP2040 Feather RFM95 built-in pins
#define RFM95_CS  16
#define RFM95_RST 17
#define RFM95_INT 21
#define RF95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("RX init FAILED");
    while (1);
  }
  rf95.setFrequency(RF95_FREQ);
  Serial.println("RX ready — listening...");
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      Serial.print("Got packet: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
    }
  }
}