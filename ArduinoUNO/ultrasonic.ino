/*
  This file contains the code to read the 4 ultrasonic sensors
  on timer 1 of UNO 
*/

#define LEFT_TRIGGER 18 // A4
#define LEFT_ECHO 19 // A5
#define RIGHT_TRIGGER 6
#define RIGHT_ECHO 7  
#define FRONT_TRIGGER 8 
#define FRONT_ECHO 9 
#define BACK_TRIGGER 16 // A2
#define BACK_ECHO 17 // A3

const int triggerPins[4] = { LEFT_TRIGGER, RIGHT_TRIGGER, FRONT_TRIGGER, BACK_TRIGGER }; 
const int echoPins[4] = { LEFT_ECHO, RIGHT_ECHO, FRONT_ECHO, BACK_ECHO };

void setupUltrasonic() {
  for (int i = 0; i < 4; i++) {
    pinMode(triggerPins[i], OUTPUT);  // Set trigger pins as output
    pinMode(echoPins[i], INPUT);      // Set echo pins as input
  }
}

void ultrasonicGetDistances() {
  // Iterate through each sensor
  for (int i = 0; i < 4; i++) {
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
