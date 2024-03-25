#include <serialize.h>
#include <stdarg.h>
#include <math.h>
#include "packet.h"
#include "constants.h"
#include "Arduino.h"

typedef enum
{
  STOP = 0,
  FORWARD = 1,
  BACKWARD = 2,
  LEFT = 3,
  RIGHT = 4
} TDirection;

volatile TDirection dir = STOP;

/*
   Alex's configuration constants
*/

// Number of ticks per revolution from the
// wheel encoder.

#define COUNTS_PER_REV 91 // 76-79 counts tested at 100 speed

// Wheel circumference in cm.
// We will use this to calculate forward/backward distance traveled
// by taking revs * WHEEL_CIRC

#define PI 3.141592654
#define WHEEL_CIRC (6.5 * PI)

// Pins for PORTC
#define s0 (1 << 3)  // A3
#define out (1 << 4) // A4
#define s1 (1 << 2)  // A2
#define s2 (1 << 1)  // A1
#define s3 (1 << 5)  // A5yeet

// Length and Breadth in cm
#define ALEX_LENGTH 12
#define ALEX_BREADTH 8

float AlexDiagonal = 0.0;
float AlexCirc = 0.0;

// Motor control pins. You need to adjust these till
// Alex moves in the correct direction
#define LF (1 << 6) // Left forward pin PORTD pin 6/OC0A
#define LR (1 << 5) // Left reverse pin PORTD pin 5/OC0B
#define RF (1 << 3) // Right forward pin PORTB  pin 3/OC2A
#define RR (1 << 2) // Right reverse pin PORTB pin 2/OC1B

/*
      Alex's State Variables
*/

// Store the ticks from Alex's left and
// right encoders.
volatile unsigned long leftForwardTicks;
volatile unsigned long rightForwardTicks;
volatile unsigned long leftReverseTicks;
volatile unsigned long rightReverseTicks;

volatile unsigned long leftForwardTicksTurns;
volatile unsigned long rightForwardTicksTurns;
volatile unsigned long leftReverseTicksTurns;
volatile unsigned long rightReverseTicksTurns;

// Store the revolutions on Alex's left
// and right wheels
volatile unsigned long leftRevs;
volatile unsigned long rightRevs;

// Forward and backward distance traveled
volatile unsigned long forwardDist;
volatile unsigned long reverseDist;
unsigned long deltaDist;
unsigned long newDist;
unsigned long deltaTicks;
unsigned long targetTicks;

// variables for Colour Sensor
double white = 0;
double red = 0;
double green = 0;
double blue = 0;
/*

   Alex Communication Routines.

*/

// Serial communication variables
unsigned char RX_Buffer[256] = {0};
volatile unsigned long RX_Buffer_Head;
volatile unsigned long RX_Buffer_Tail;
volatile unsigned int RX_Bytes;

unsigned char TX_Buffer[256] = {0};
volatile unsigned long TX_Buffer_Head;
volatile unsigned long TX_Buffer_Tail;
volatile unsigned int TX_Bytes;

TResult readPacket(TPacket *packet)
{
  // Reads in data from the serial port and
  // deserializes it.Returns deserialized
  // data in "packet".

  char buffer[PACKET_SIZE];
  int len;

  len = readSerial(buffer);

  if (len == 0)
    return PACKET_INCOMPLETE;
  else
    return deserialize(buffer, len, packet);
}

void colour()
{
  // put your main code here, to run repeatedly:

  PORTC |= s2;
  PORTC &= ~s3;
  white = (double)pulseIn(out, LOW);
  delay(20);

  PORTC &= ~s2;
  PORTC &= ~s3;
  red = (white / (double)pulseIn(out, LOW));
  delay(20);

  PORTC &= ~s2;
  PORTC |= s3;
  blue = (white / (double)pulseIn(out, LOW));
  delay(20);

  PORTC |= s2;
  PORTC |= s3;
  green = (white / (double)pulseIn(out, LOW));
  delay(20);
}

void sendStatus()
{
  // Implement code to send back a packet containing key
  // information like leftTicks, rightTicks, leftRevs, rightRevs
  // forwardDist and reverseDist
  // Use the params array to store this information, and set the
  // packetType and command files accordingly, then use sendResponse
  // to send out the packet. See sendMessage on how to use sendResponse.
  //
  TPacket statusPacket;
  sendOK();
  colour();
  statusPacket.packetType = PACKET_TYPE_RESPONSE;
  statusPacket.command = RESP_STATUS;
  statusPacket.params[0] = leftForwardTicks;
  statusPacket.params[1] = rightForwardTicks;
  statusPacket.params[2] = leftReverseTicks;
  statusPacket.params[3] = rightReverseTicks;
  statusPacket.params[4] = leftForwardTicksTurns;
  statusPacket.params[5] = rightForwardTicksTurns;
  statusPacket.params[6] = leftReverseTicksTurns;
  statusPacket.params[7] = rightReverseTicksTurns;
  statusPacket.params[8] = forwardDist;
  statusPacket.params[9] = reverseDist;
  statusPacket.params[10] = (uint32_t)(red * 100);
  statusPacket.params[11] = (uint32_t)(green * 100);
  statusPacket.params[12] = (uint32_t)(blue * 100);

  sendResponse(&statusPacket);
}

void sendMessage(const char *message)
{
  // Sends text messages back to the Pi. Useful
  // for debugging.

  TPacket messagePacket;
  messagePacket.packetType = PACKET_TYPE_MESSAGE;
  strncpy(messagePacket.data, message, MAX_STR_LEN);
  sendResponse(&messagePacket);
}

void dbprintf(char *format, ...)
{
  va_list args;
  char buffer[128];

  va_start(args, format);
  vsprintf(buffer, format, args);
  sendMessage(buffer);
}

void sendBadPacket()
{
  // Tell the Pi that it sent us a packet with a bad
  // magic number.

  TPacket badPacket;
  badPacket.packetType = PACKET_TYPE_ERROR;
  badPacket.command = RESP_BAD_PACKET;
  sendResponse(&badPacket);
}

void sendBadChecksum()
{
  // Tell the Pi that it sent us a packet with a bad
  // checksum.

  TPacket badChecksum;
  badChecksum.packetType = PACKET_TYPE_ERROR;
  badChecksum.command = RESP_BAD_CHECKSUM;
  sendResponse(&badChecksum);
}

void sendBadCommand()
{
  // Tell the Pi that we don't understand its
  // command sent to us.

  TPacket badCommand;
  badCommand.packetType = PACKET_TYPE_ERROR;
  badCommand.command = RESP_BAD_COMMAND;
  sendResponse(&badCommand);
}

void sendBadResponse()
{
  TPacket badResponse;
  badResponse.packetType = PACKET_TYPE_ERROR;
  badResponse.command = RESP_BAD_RESPONSE;
  sendResponse(&badResponse);
}

void sendOK()
{
  TPacket okPacket;
  okPacket.packetType = PACKET_TYPE_RESPONSE;
  okPacket.command = RESP_OK;
  sendResponse(&okPacket);
}

void sendResponse(TPacket *packet)
{
  // Takes a packet, serializes it then sends it out
  // over the serial port.
  char buffer[PACKET_SIZE];
  int len;

  len = serialize(buffer, packet, sizeof(TPacket));
  writeSerial(buffer, len);
}

/*
   Setup and start codes for external interrupts and
   pullup resistors.

*/
// Enable pull up resistors on pins 2 and 3
void enablePullups()
{
  // Use bare-metal to enable the pull-up resistors on pins
  // 2 and 3. These are pins PD2 and PD3 respectively.
  // We set bits 2 and 3 in DDRD to 0 to make them inputs.

  DDRD &= 0b11110011;
  PORTD |= 0b00001100;
}

// Functions to be called by INT0 and INT1 ISRs.
void leftISR()
{
  switch (dir)
  {
  case (FORWARD):
    leftForwardTicks++;
    forwardDist = (unsigned long)((float)leftForwardTicks / COUNTS_PER_REV * WHEEL_CIRC);
    break;
  case (BACKWARD):
    leftReverseTicks++;
    reverseDist = (unsigned long)((float)leftReverseTicks / COUNTS_PER_REV * WHEEL_CIRC);
    break;
  case (LEFT):
    leftReverseTicksTurns++;
    break;
  case (RIGHT):
    leftForwardTicksTurns++;
    break;
  };
}

void rightISR()
{
  switch (dir)
  {
  case (FORWARD):
    rightForwardTicks++;
    break;
  case (BACKWARD):
    rightReverseTicks++;
    break;
  case (LEFT):
    rightForwardTicksTurns++;
    break;
  case (RIGHT):
    rightReverseTicksTurns++;
    break;
  };
}

// Set up the external interrupt pins INT0 and INT1
// for falling edge triggered. Use bare-metal.
void setupEINT()
{
  // Use bare-metal to configure pins 2 and 3 to be
  // falling edge triggered. Remember to enable
  // the INT0 and INT1 interrupts.
  EICRA = 0b00001010;
  EIMSK = 0b00000011;
}

// Implement the external interrupt ISRs below.
// INT0 ISR should call leftISR while INT1 ISR
// should call rightISR.

ISR(INT0_vect)
{
  leftISR();
}

ISR(INT1_vect)
{
  rightISR();
}

// Implement INT0 and INT1 ISRs above.

/*
   Setup and start codes for serial communications

*/
// Set up the serial connection. For now we are using
// Arduino Wiring, you will replace this later
// with bare-metal code.
void setupSerial()
{
  // To replace later with bare-metal.
  UCSR0A = 0;
  UCSR0C = 0b00000110;
  UBRR0L = 103;
  UBRR0H = 0;
  RX_Buffer_Head = 0;
  RX_Buffer_Tail = 0;
  TX_Buffer_Head = 0;
  TX_Buffer_Tail = 0;
  TX_Bytes = 0;
}

// Start the serial connection. For now we are using
// Arduino wiring and this function is empty. We will
// replace this later with bare-metal code.

void startSerial()
{
  // Empty for now. To be replaced with bare-metal code
  // later on.

  UCSR0B = 0b11011000;
}

// Read the serial port. Returns the read character in
// ch if available. Also returns TRUE if ch is valid.
// This will be replaced later with bare-metal code.

// writes a char to the RX buffer
void Write_RX_Buffer(unsigned char data)
{
  RX_Buffer[RX_Buffer_Tail] = data;
  RX_Buffer_Tail = (RX_Buffer_Tail + 1) % 256;
  RX_Bytes += 1;
}

// reads a char from the RX buffer
unsigned char Read_RX_Buffer()
{
  unsigned char data;
  data = RX_Buffer[RX_Buffer_Head];
  RX_Buffer_Head = (RX_Buffer_Head + 1) % 256;
  RX_Bytes--;
  return data;
}

ISR(USART_RX_vect)
{
  unsigned char data = UDR0;
  if (RX_Bytes < 256)
  {
    Write_RX_Buffer(data);
  }
}

int readSerial(char *buffer)
{

  int count = 0;

  while (RX_Bytes > 0)
  {
    buffer[count++] = Read_RX_Buffer();
  }

  return count;
}

// TX buffer functions

// Write a char to the TX buffer
void Write_TX_Buffer(char data)
{
  TX_Buffer[TX_Buffer_Tail] = data;
  TX_Buffer_Tail = (TX_Buffer_Tail + 1) % 256;
  TX_Bytes += 1;
}

// Read a char from the TX buffer
unsigned char Read_TX_Buffer()
{
  unsigned char data = 0;
  data = TX_Buffer[TX_Buffer_Head];
  TX_Buffer_Head = (TX_Buffer_Head + 1) % 256;
  TX_Bytes -= 1;

  return data;
}

ISR(USART_TX_vect)
{

  if (TX_Bytes != 0)
  {
    UDR0 = Read_TX_Buffer();
  }

  // disable interrupts if no data to be sent
  else
  {
    UCSR0B &= ~(1 << 6);
  }
}

// Write to the serial port.

void writeSerial(const char *buffer, int len)
{

  for (int count = 0; count < len && TX_Bytes < 256; count += 1)
  {
    Write_TX_Buffer(buffer[count]);
  }

  UCSR0B |= (1 << 6);
}

void setupMotors()
{
  /* Our motor set up is:
        A1IN - Pin 5, PD5, OC0B LF
        A2IN - Pin 6, PD6, OC0A LR
        B1IN - Pin 10, PB2, OC1B RF
        B2In - pIN 11, PB3, OC2A RR
  */
  DDRD |= (LF | LR);
  DDRB |= (RF | RR);
  TCCR0A |= (1 << WGM00) | (1 << COM0A1) | (1 << COM0B1) ;
  TCCR0B |= (1 << CS01);
  TCCR1A |= (1 << WGM10) | (1 << COM1B1);
  TCCR1B |= (1 << CS11);
  TCCR2A |= (1 << WGM20) | (1 << COM2A1);
  TCCR2B |= (1 << CS21);

  TIMSK1 |= (1 << 2);
  TIMSK0 |= (1 << 2) | (1 << 1);
  TIMSK2 |= (1 << 1);
}

// Start the PWM for Alex's motors.
// We will implement this later. For now it is
// blank.
void startMotors()
{
  PORTD |= LF;
  PORTD &= ~LR;
  PORTB |= RF;
  PORTB &= ~RR;
}

// Convert percentages to PWM values
int pwmVal(float speed)
{
  if (speed < 0.0)
    speed = 0;

  if (speed > 100.0)
    speed = 100.0;

  return (int)((speed / 100.0) * 255.0);
}

// Move Alex forward "dist" cm at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// move forward at half speed.
// Specifying a distance of 0 means Alex will
// continue moving forward indefinitely.
void forward(float dist, float speed)
{
  dir = FORWARD;

  int val = pwmVal(speed);

  if (dist == 0)
  {
    deltaDist = 999999;
  }
  else
  {
    deltaDist = dist;
  }

  newDist = forwardDist + deltaDist;

  // Left forward pin PORTD pin 6/OC0A
  // Left reverse pin PORTD pin 5/OC0B
  // Right forward pin PORTB  pin 3/OC2A
  // Right reverse pin PORTB pin 2/OC1B
  OCR0A = val;
  OCR0B = 0;
  OCR2A = val;
  OCR1B = 0;
}

// Reverse Alex "dist" cm at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// reverse at half speed.
// Specifying a distance of 0 means Alex will
// continue reversing indefinitely.
void reverse(float dist, float speed)
{
  dir = BACKWARD;

  int val = pwmVal(speed);

  if (dist == 0)
  {
    deltaDist = 999999;
  }
  else
  {
    deltaDist = dist;
  }

  newDist = reverseDist + deltaDist;

  // Left forward pin PORTD pin 6/OC0A
  // Left reverse pin PORTD pin 5/OC0B
  // Right forward pin PORTB  pin 3/OC2A
  // Right reverse pin PORTB pin 2/OC1B
  OCR0A = 0;
  OCR0B = val;
  OCR2A = 0;
  OCR1B = val;
}

unsigned long computeDeltaTicks(float ang)
{
  unsigned long ticks = (unsigned long)((ang * AlexCirc * COUNTS_PER_REV) / (360.0 * WHEEL_CIRC));

  return ticks;
}
// Turn Alex left "ang" degrees at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// turn left at half speed.
// Specifying an angle of 0 degrees will cause Alex to
// turn left indefinitely.
void left(float ang, float speed)
{
  dir = LEFT;

  if (ang == 0)
  {
    deltaTicks = 99999999;
  }
  else
  {
    deltaTicks = computeDeltaTicks(ang);
  }

  targetTicks = leftReverseTicksTurns + deltaTicks;

  int val = pwmVal(speed);

  // Left forward pin PORTD pin 6/OC0A
  // Left reverse pin PORTD pin 5/OC0B
  // Right forward pin PORTB  pin 3/OC2A
  // Right reverse pin PORTB pin 2/OC1B
  OCR0A = 0;
  OCR0B = val;
  OCR2A = val;
  OCR1B = 0;
}

// Turn Alex right "ang" degrees at speed "speed".
// "speed" is expressed as a percentage. E.g. 50 is
// turn left at half speed.
// Specifying an angle of 0 degrees will cause Alex to
// turn right indefinitely.
void right(float ang, float speed)
{
  dir = RIGHT;

  if (ang == 0)
  {
    deltaTicks = 99999999;
  }
  else
  {
    deltaTicks = computeDeltaTicks(ang);
  }

  targetTicks = rightReverseTicksTurns + deltaTicks;

  int val = pwmVal(speed);

  // Left forward pin PORTD pin 6/OC0A
  // Left reverse pin PORTD pin 5/OC0B
  // Right forward pin PORTB  pin 3/OC2A
  // Right reverse pin PORTB pin 2/OC1B
  OCR0A = val;
  OCR0B = 0;
  OCR2A = 0;
  OCR1B = val;
}

// Stop Alex. To replace with bare-metal code later.
void stop()
{
  dir = STOP;

  OCR0A = 0;
  OCR0B = 0;
  OCR2A = 0;
  OCR1B = 0;
}

/*
   Alex's setup and run codes

*/

// Clears all our counters
void clearCounters()
{
  leftForwardTicks = 0;
  rightForwardTicks = 0;
  leftReverseTicks = 0;
  rightReverseTicks = 0;
  leftForwardTicksTurns = 0;
  rightForwardTicksTurns = 0;
  leftReverseTicksTurns = 0;
  rightReverseTicksTurns = 0;
  leftRevs = 0;
  rightRevs = 0;
  forwardDist = 0;
  reverseDist = 0;
}

// Clears one particular counter
void clearOneCounter(int which)
{
  clearCounters();
}
// Intialize Vincet's internal states

void initializeState()
{
  clearCounters();
}

void handleCommand(TPacket *command)
{
  switch (command->command)
  {
  // For movement commands, param[0] = distance, param[1] = speed.
  case COMMAND_FORWARD:
    sendOK();
    forward((float)command->params[0], (float)command->params[1]);
    break;

  case COMMAND_REVERSE:
    sendOK();
    reverse((float)command->params[0], (float)command->params[1]);
    break;

  case COMMAND_TURN_LEFT:
    sendOK();
    left((float)command->params[0], (float)command->params[1]);
    break;

  case COMMAND_TURN_RIGHT:
    sendOK();
    right((float)command->params[0], (float)command->params[1]);
    break;

  case COMMAND_STOP:
    sendOK();
    stop();
    break;

  case COMMAND_GET_STATS:
    sendStatus();
    break;

  case COMMAND_CLEAR_STATS:
    sendOK();
    clearOneCounter(command->params[0]);
    break;

  default:
    sendBadCommand();
  }
}

void waitForHello()
{
  int exit = 0;

  while (!exit)
  {
    TPacket hello;
    TResult result;

    do
    {
      result = readPacket(&hello);
    } while (result == PACKET_INCOMPLETE);

    if (result == PACKET_OK)
    {
      if (hello.packetType == PACKET_TYPE_HELLO)
      {

        sendOK();
        exit = 1;
      }
      else
        sendBadResponse();
    }
    else if (result == PACKET_BAD)
    {
      sendBadPacket();
    }
    else if (result == PACKET_CHECKSUM_BAD)
      sendBadChecksum();
  } // !exit
}

void setup()
{
  // put your setup code here, to run once:

  // compute the diagonal
  AlexDiagonal = sqrt((ALEX_LENGTH * ALEX_LENGTH) + (ALEX_BREADTH * ALEX_BREADTH));
  AlexCirc = PI * AlexDiagonal;
  DDRC |= (s0 | s1 | s2 | s3);
  DDRC &= ~(out);
  PORTC |= (s0 | s1);

  cli();
  setupEINT();
  setupSerial();
  startSerial();
  setupMotors();
  startMotors();
  enablePullups();
  initializeState();
  sei();
}

void handlePacket(TPacket *packet)
{
  switch (packet->packetType)
  {
  case PACKET_TYPE_COMMAND:
    handleCommand(packet);
    break;

  case PACKET_TYPE_RESPONSE:
    break;

  case PACKET_TYPE_ERROR:
    break;

  case PACKET_TYPE_MESSAGE:
    break;

  case PACKET_TYPE_HELLO:
    break;
  }
}

void loop()
{

  TPacket recvPacket; // This holds commands from the Pi

  TResult result = readPacket(&recvPacket);

  if (result == PACKET_OK)
    handlePacket(&recvPacket);
  else if (result == PACKET_BAD)
  {
    sendBadPacket();
  }
  else if (result == PACKET_CHECKSUM_BAD)
  {
    sendBadChecksum();
  }

  if (deltaDist > 0)
  {
    if (dir == FORWARD)
    {
      if (forwardDist >= newDist)
      {
        deltaDist = 0;
        newDist = 0;
        stop();
      }
    }
    else if (dir == BACKWARD)
    {
      if (reverseDist >= newDist)
      {
        deltaDist = 0;
        newDist = 0;
        stop();
      }
    }
    else if (dir == STOP)
    {
      deltaDist = 0;
      newDist = 0;
      stop();
    }
  }

  if (deltaTicks > 0)
  {

    if (dir == LEFT)
    {
      if (leftReverseTicksTurns >= targetTicks)
      {
        deltaTicks = 0;
        targetTicks = 0;
        stop();
      }
    }

    else if (dir == RIGHT)
    {
      if (rightReverseTicksTurns >= targetTicks)
      {
        deltaTicks = 0;
        targetTicks = 0;
        stop();
      }
    }

    else if (dir == STOP)
    {
      deltaTicks = 0;
      targetTicks = 0;
      stop();
    }
  }
}
