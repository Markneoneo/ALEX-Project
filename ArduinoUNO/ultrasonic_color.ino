#define NUM_SENSORS 4
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

void setupTimer() {
  // Configure Timer1
  TCCR1A = 0; // Normal mode
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC mode, prescaler 1024
  OCR1A = 15625; // Set compare value for 1 second delay at 16MHz with prescaler 1024
  TIMSK1 = (1 << OCIE1A); // Enable Timer1 compare match A interrupt
  
  sei();  // Enable global interrupts
}

ISR(TIMER1_COMPA_vect) {
  // This ISR will be called every 1 second
  if (serialOn) {
    Serial.println("1 second delay completed");
  }
  ultrasonicGetDistances();
  color_check();
  // Clear Timer1
  TCNT1 = 0;
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
