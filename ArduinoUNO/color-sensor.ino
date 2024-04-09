/*

TCS3200 Color Sensor

Pin Name         I/O	     Description
GND (4)		                 Power supply ground
OE (3)	          I	       Enable for output frequency (active low)
OUT (6)	          O	       Output frequency 
S0, S1（1, 2)     I	       Output frequency scaling selection inputs
S2, S3（7, 8)     I	       Photodiode type selection inputs
VDD（5）		               Voltage supply

Filter selection:
To select the color read by the photodiode, you use the control pins S2 and S3.
Photodiode type     S2	    S3
Red	                LOW	    LOW
Blue	              LOW	    HIGH
Clear	              HIGH	  LOW
Green	              HIGH	  HIGH

Frequency scaling:
Pins S0 and S1 are used for scaling the output frequency.
Output frequency scaling	S0	S1
Power down	              L	  L
2%	                      L	  H
20%	                      H	  L
100%	                    H	  H

*/

// TCS230 or TCS3200 pins wiring to Arduino
#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define Out 4

// Stores frequency read by the photodiodes
int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

// Stores the red, green and blue colors
int redColor = 0; // 1
int greenColor = 0; // 2
int blueColor = 0; // 3
int color = 0; 

void setupColor() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(Out, INPUT);

  // Setting frequency scaling to 100%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
}

void color_check() {
  // Wait until "out" go LOW, Start measuring the duration, Stop when "out" is HIGH again
  // Setting RED (R) filtered photodiodes to be read
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  redFrequency = pulseIn(Out, LOW);
  // Remaping the value of the RED (R) frequency from 0 to 255
  redColor = map(redFrequency, 9, 80, 255, 0); // 10/80
  // Printing the RED (R) value
  if (serialOn) {
    Serial.print("R = ");
    Serial.print(redColor);
  }
  delay(100);

  // Setting GREEN (G) filtered photodiodes to be read
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  greenFrequency = pulseIn(Out, LOW); 
  // Remaping the value of the GREEN (G) frequency from 0 to 255
  greenColor = map(greenFrequency, 16, 75, 255, 0); // 15/75
  // Printing the GREEN (G) value  
  if (serialOn) {
    Serial.print(" G = ");
    Serial.print(greenColor);
  }
  delay(100);

  // Setting BLUE (B) filtered photodiodes to be read
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  blueFrequency = pulseIn(Out, LOW);
  // Remaping the value of the BLUE (B) frequency from 0 to 255
  blueColor = map(blueFrequency, 8, 63, 255, 0); //10/65
  // Printing the BLUE (B) value 
  if (serialOn) {
    Serial.print(" B = ");
    Serial.println(blueColor);
  }
  delay(100);

  int threshold = 30;

  // Checks the current detected color and prints a message in the serial monitor
  if(abs(redColor - greenColor) <= threshold && abs(redColor - blueColor) <= threshold && abs(greenColor - blueColor) <= threshold){
    color = 0;
    if (serialOn) {
      Serial.println(" - WHITE detected!");
    }
  } else if(redColor > greenColor && redColor > blueColor){
    color = 1;
    if (serialOn) {
      Serial.println(" - RED detected!");
    }
  } else if(greenColor > redColor && greenColor > blueColor){
    color = 2;
    if (serialOn) {
      Serial.println(" - GREEN detected!");
    }
  //} else if(blueColor > redColor && blueColor > greenColor){
    //color = 3;
    //Serial.println(" - BLUE detected!");
  } else {
      if (serialOn) {
        Serial.println(" - OTHER detected!");
      }
    color = 0;
  }
  delay(200);
}
