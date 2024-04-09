#define NUM_SENSORS 4
bool serialOn = true;
// Array to store distance readings
volatile uint32_t ultrasonicDistances[NUM_SENSORS] = { 0 }; 

const int triggerPins[NUM_SENSORS] = { 6, 8, 10, 12 };
const int echoPins[NUM_SENSORS] = { 7, 9, 11, 13 };

void setupUltrasonic() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(triggerPins[i], OUTPUT);  // Set trigger pins as output
    pinMode(echoPins[i], INPUT);      // Set echo pins as input
  }
}

void ultrasonicGetDistances() {
  // Iterate through each sensor
  for (int i = 0; i < NUM_SENSORS; i++) {
    long duration;
    int distance;

    // Clear the trigger pin
    digitalWrite(triggerPins[i], LOW);
    delayMicroseconds(2);

    // Send a 10 microsecond pulse to trigger pin
    digitalWrite(triggerPins[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPins[i], LOW);

    // Read the pulse on echo pin
    duration = pulseIn(echoPins[i], HIGH);

    // Convert the pulse duration to distance
    distance = duration * 0.034 / 2;  // Speed of sound is 0.034 cm/μs

    // Print the distance for each sensor
    if (serialOn) {
      Serial.print("Sensor ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(distance);
      Serial.println(" cm");
    }

    // Store the distance in the ultrasonicDistances array
    ultrasonicDistances[i] = distance;
  }
}

void setup() {
  setupUltrasonic();
  Serial.begin(9600);
}

void loop() {
  ultrasonicGetDistances();
  delay(1000);
}
