#include <inttypes.h>
#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #if defined(__AVR__)
    #include <avr/io.h>
  #endif
  #include "WProgram.h"
#endif

#define MOTOR1_A 2//FR
#define MOTOR1_B 3
#define MOTOR2_A 1//BR
#define MOTOR2_B 4
#define MOTOR4_A 0//FL
#define MOTOR4_B 6
#define MOTOR3_A 5//BL
#define MOTOR3_B 7

#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

#define LATCH 4
#define LATCH_DDR DDRB
#define LATCH_PORT PORTB

#define CLK_PORT PORTD
#define CLK_DDR DDRD
#define CLK 4

#define ENABLE_PORT PORTD
#define ENABLE_DDR DDRD
#define ENABLE 7

#define SER 0
#define SER_DDR DDRB
#define SER_PORT PORTB

#define MOTOR12_8KHZ _BV(CS21)              // divide by 8
#define MOTOR12_2KHZ _BV(CS21) | _BV(CS20)  // divide by 32
#define MOTOR12_1KHZ _BV(CS22)              // divide by 64

#define MOTOR34_64KHZ _BV(CS00)             // no prescale
#define MOTOR34_8KHZ _BV(CS01)              // divide by 8
#define MOTOR34_1KHZ _BV(CS01) | _BV(CS00)  // divide by 64
    
#define DC_MOTOR_PWM_RATE   MOTOR34_8KHZ    // PWM rate for DC motors


static uint8_t latch_state;




void latch_tx(void) { //send data to shift register
  uint8_t i;

  LATCH_PORT &= ~_BV(LATCH);

  SER_PORT &= ~_BV(SER);

  for (i=0; i<8; i++) {
    CLK_PORT &= ~_BV(CLK);

    if (latch_state & _BV(7-i)) {
      SER_PORT |= _BV(SER);
    } else {
      SER_PORT &= ~_BV(SER);
    }
    CLK_PORT |= _BV(CLK);
  }
  LATCH_PORT |= _BV(LATCH);
}


void enable(void) {
  // setup the latch
  
  LATCH_DDR |= _BV(LATCH);
  ENABLE_DDR |= _BV(ENABLE);
  CLK_DDR |= _BV(CLK);
  SER_DDR |= _BV(SER);
  
  latch_state = 0;

  latch_tx();  // "reset"

  ENABLE_PORT &= ~_BV(ENABLE); // enable the chip outputs!

}




/******************************************
               MOTORS
******************************************/
inline void initPWM1(uint8_t freq) {
    // on arduino mega, pin 11 is now PB5 (OC1A)
    TCCR1A |= _BV(COM1A1) | _BV(WGM10); // fast PWM, turn on oc1a
    TCCR1B = (freq & 0x7) | _BV(WGM12);
    OCR1A = 0;
    DDRD |= _BV(5);
}

inline void setPWM1(uint8_t s) {
    // on arduino mega, pin 11 is now PB5 (OC1A)
    OCR1A = s;
}

inline void initPWM2(uint8_t freq) {
    // on arduino mega, pin 3 is now PE5 (OC3C)
    TCCR3A |= _BV(COM1C1) | _BV(WGM10); // fast PWM, turn on oc3c
    TCCR3B = (freq & 0x7) | _BV(WGM12);
    OCR3C = 0;
    DDRE = _BV(5);
}

inline void setPWM2(uint8_t s) {
    // on arduino mega, pin 11 is now PB5 (OC1A)
    OCR3C = s;
}

inline void initPWM3(uint8_t freq) {

    // on arduino mega, pin 6 is now PH3 (OC4A)
    TCCR4A |= _BV(COM1A1) | _BV(WGM10); // fast PWM, turn on oc4a
    TCCR4B = (freq & 0x7) | _BV(WGM12);
    //TCCR4B = 1 | _BV(WGM12);
    OCR4A = 0;
    DDRH |= _BV(3);
}

inline void setPWM3(uint8_t s) {
    OCR4A = s;
}



inline void initPWM4(uint8_t freq) {
    // on arduino mega, pin 5 is now PE3 (OC3A)
    TCCR3A |= _BV(COM1A1) | _BV(WGM10); // fast PWM, turn on oc3a
    TCCR3B = (freq & 0x7) | _BV(WGM12);
    //TCCR4B = 1 | _BV(WGM12);
    OCR3A = 0;
    DDRE |= _BV(3);

}

inline void setPWM4(uint8_t s) {
    // on arduino mega, pin 6 is now PH3 (OC4A)
    OCR3A = s;
}

void startMotor() {
  enable();
  latch_state &= ~_BV(MOTOR1_A) & ~_BV(MOTOR1_B); // set both motor pins to 0
  latch_state &= ~_BV(MOTOR2_A) & ~_BV(MOTOR2_B); // set both motor pins to 0
  latch_state &= ~_BV(MOTOR3_A) & ~_BV(MOTOR3_B); // set both motor pins to 0
  latch_state &= ~_BV(MOTOR4_A) & ~_BV(MOTOR4_B); // set both motor pins to 0
  latch_tx();
  initPWM1(_BV(1));
  initPWM2(_BV(1));
  initPWM3(_BV(1));
  initPWM4(_BV(1));
}

void runFL(uint8_t cmd) {
  uint8_t a = MOTOR4_A; 
  uint8_t b = MOTOR4_B;
  switch (cmd) {
  case FORWARD:
    latch_state |= _BV(a);
    latch_state &= ~_BV(b); 
    latch_tx();
    break;
  case BACKWARD:
    latch_state &= ~_BV(a);
    latch_state |= _BV(b); 
    latch_tx();
    break;
  case RELEASE:
    latch_state &= ~_BV(a);     // A and B both low
    latch_state &= ~_BV(b); 
    latch_tx();
    break;
  }
}

void runFR(uint8_t cmd) {
  uint8_t a = MOTOR1_A; 
  uint8_t b = MOTOR1_B;
  switch (cmd) {
  case FORWARD:
    latch_state |= _BV(a);
    latch_state &= ~_BV(b); 
    latch_tx();
    break;
  case BACKWARD:
    latch_state &= ~_BV(a);
    latch_state |= _BV(b); 
    latch_tx();
    break;
  case RELEASE:
    latch_state &= ~_BV(a);     // A and B both low
    latch_state &= ~_BV(b); 
    latch_tx();
    break;
  }
}

void runBL(uint8_t cmd) {
  uint8_t a = MOTOR3_A; 
  uint8_t b = MOTOR3_B;
  switch (cmd) {
  case FORWARD:
    latch_state |= _BV(a);
    latch_state &= ~_BV(b); 
    latch_tx();
    break;
  case BACKWARD:
    latch_state &= ~_BV(a);
    latch_state |= _BV(b); 
    latch_tx();
    break;
  case RELEASE:
    latch_state &= ~_BV(a);     // A and B both low
    latch_state &= ~_BV(b); 
    latch_tx();
    break;
  }
}

void runBR(uint8_t cmd) {
  uint8_t a = MOTOR2_A; 
  uint8_t b = MOTOR2_B;
  switch (cmd) {
  case FORWARD:
    latch_state |= _BV(a);
    latch_state &= ~_BV(b); 
    latch_tx();
    break;
  case BACKWARD:
    latch_state &= ~_BV(a);
    latch_state |= _BV(b); 
    latch_tx();
    break;
  case RELEASE:
    latch_state &= ~_BV(a);     // A and B both low
    latch_state &= ~_BV(b); 
    latch_tx();
    break;
  }
}

void setSpeed(uint8_t speed) {
    setPWM1(speed);
    setPWM2(speed);
    setPWM3(speed);
    setPWM4(speed);
}

