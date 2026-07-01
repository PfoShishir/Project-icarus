// Geiger Counter - Count radiation clicks!

volatile int clicks = 0;

void boop() {
  clicks++;  // every radiation click, add 1
}

void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), boop, FALLING);
  Serial.println("Geiger Counter Ready! Counting clicks...");
}

void loop() {
  delay(60000);  // wait 10 seconds

  Serial.print("Clicks in 1 min: ");
  Serial.println(clicks);
  clicks = 0;  // reset and start again
}