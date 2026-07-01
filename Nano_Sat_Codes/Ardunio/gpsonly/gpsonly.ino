//GPS code only works fine
#include <SoftwareSerial.h>

// GPS serial (RX, TX)
SoftwareSerial gpsSerial(8, 7);

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);

  Serial.println("GPS Test Starting...");
}

void loop() {

  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    Serial.write(c);
  
  
   
  }
}