bool serialOn = true;

void setup() {
  setupTimer();
  setupColor();
  setupUltrasonic();
  Serial.begin(9600);
}

void loop() {
  // No loop as the interval is triggered by ISR
}
