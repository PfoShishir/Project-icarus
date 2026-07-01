//sender
#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS  4
#define RFM95_RST 2
#define RFM95_INT 3
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
    Serial.println("TX init FAILED");
    while (1);
  }
  rf95.setFrequency(RF95_FREQ);
  rf95.setTxPower(13, false);
  Serial.println("TX ready — sending packets...");
}

void loop() {
  const char* msg = "NANOSAT PING";
  rf95.send((uint8_t*)msg, strlen(msg));
  rf95.waitPacketSent();
  Serial.println("Packet sent!");
  delay(1000);
}