#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MCP9808.h>

Adafruit_BMP280 bmp;
Adafruit_MCP9808 mcp;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  Serial.println(F("BMP280 + MCP9808 Test Starting..."));

  // BMP280 init
  if (!bmp.begin(0x77)) {
    Serial.println(F("BMP280 NOT found!"));
    while (1);
  }
  Serial.println(F("BMP280 found"));

  // BMP280 settings
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);

  // MCP9808 init
  if (!mcp.begin(0x18)) {
    Serial.println(F("MCP9808 NOT found!"));
    while (1);
  }
  Serial.println(F("MCP9808 found"));

  // Set MCP9808 resolution
  // 0 = low  (0.5°C)
  // 1 = med  (0.25°C)
  // 2 = high (0.125°C)
  // 3 = max  (0.0625°C)
  mcp.setResolution(3);

  Serial.println(F("All sensors ready!"));
  Serial.println(F("------------------------------------"));
}

void loop() {
  // Read BMP280
  float bmpTemp  = bmp.readTemperature();
  float pressPA  = bmp.readPressure();
  float altM     = bmp.readAltitude(1013.25);

  // Read MCP9808
  float mcpTemp  = mcp.readTempC();

  // Print results
  Serial.println(F("----------"));

  Serial.println(F("BMP280:"));
  Serial.print(F("  Temperature: ")); Serial.print(bmpTemp); Serial.println(F(" C"));
  Serial.print(F("  Pressure:    ")); Serial.print(pressPA); Serial.println(F(" Pa"));
  Serial.print(F("  Altitude:    ")); Serial.print(altM);    Serial.println(F(" m"));

  Serial.println(F("MCP9808:"));
  Serial.print(F("  Temperature: ")); Serial.print(mcpTemp); Serial.println(F(" C"));

  // Compare both temperatures
  float diff = mcpTemp - bmpTemp;
  Serial.print(F("Temp Difference: ")); 
  Serial.print(diff); 
  Serial.println(F(" C"));

  delay(5000);
}