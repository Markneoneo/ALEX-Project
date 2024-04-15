#include <serialize.h>
#include <stdarg.h>
#include <math.h>
#include <SPI.h>
#include "packet.h"
#include "constants.h"

bool serialOn = false;

int redColor = 0;    // 1
int greenColor = 0;  // 2
int blueColor = 0;   // 0
unsigned char color = 0;

volatile unsigned char data[8] = { 0 };  // Array to store distance readings, then RGB, and colour

void setup() {
  cli(); // Prevent interrupts from interfering with setups
  //setupTimer();
  setupColor();
  setupUltrasonic();
  Serial.begin(9600);
  sei(); // Enable globsl interrupts

  // Have to send on master in, *slave out*
  pinMode(MISO, OUTPUT);
  // turn on SPI in slave mode
  SPCR |= _BV(SPE);
  // enable its interrupt
  SPI.attachInterrupt();

}

void loop() {
  color_check();
  ultrasonicGetDistances();
}

void setupTimer() {
  // Configure Timer1
  TCCR1A = 0;                                         // Normal mode
  TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);  // CTC mode, prescaler 1024
  OCR1A = 15625;                                      // Set compare value for 1 second delay at 16MHz with prescaler 1024
  TIMSK1 = (1 << OCIE1A);                             // Enable Timer1 compare match A interrupt
}
/*
  void sendStatus() {
  // Send all the info via UART to Pi
  TPacket data;
  data.packetType = PACKET_TYPE_RESPONSE;
  data.command = RESP_STATUS;
  data.params[0] = color;
  for (int i = 0; i < 4; i++) {
    data.params[i + 1] = data[i];
  }
  sendResponse(&data);
  }
*/
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

TResult readPacket(TPacket *packet) {
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

void sendMessage(const char *message) {
  // Sends text messages back to the Pi. Useful
  // for debugging.

  TPacket messagePacket;
  messagePacket.packetType = PACKET_TYPE_MESSAGE;
  strncpy(messagePacket.data, message, MAX_STR_LEN);
  sendResponse(&messagePacket);
}

void dbprintf(char *format, ...) {
  va_list args;
  char buffer[128];
  va_start(args, format);
  vsprintf(buffer, format, args);
  sendMessage(buffer);
}

void sendBadPacket() {
  // Tell the Pi that it sent us a packet with a bad magic number.

  TPacket badPacket;
  badPacket.packetType = PACKET_TYPE_ERROR;
  badPacket.command = RESP_BAD_PACKET;
  sendResponse(&badPacket);
}

void sendBadChecksum() {
  // Tell the Pi that it sent us a packet with a bad checksum.

  TPacket badChecksum;
  badChecksum.packetType = PACKET_TYPE_ERROR;
  badChecksum.command = RESP_BAD_CHECKSUM;
  sendResponse(&badChecksum);
}

void sendBadCommand() {
  // Tell the Pi that we don't understand its
  // command sent to us.

  TPacket badCommand;
  badCommand.packetType = PACKET_TYPE_ERROR;
  badCommand.command = RESP_BAD_COMMAND;
  sendResponse(&badCommand);
}

void sendBadResponse() {
  TPacket badResponse;
  badResponse.packetType = PACKET_TYPE_ERROR;
  badResponse.command = RESP_BAD_RESPONSE;
  sendResponse(&badResponse);
}

void sendOK() {
  TPacket okPacket;
  okPacket.packetType = PACKET_TYPE_RESPONSE;
  okPacket.command = RESP_OK;
  sendResponse(&okPacket);
}

void sendResponse(TPacket *packet) {
  // Takes a packet, serializes it then sends it out
  // over the serial port.
  char buffer[PACKET_SIZE];
  int len;

  len = serialize(buffer, packet, sizeof(TPacket));
  writeSerial(buffer, len);
}

int readSerial(char *buffer) {

  int count = 0;

  // Change Serial to Serial2/Serial3/Serial4 in later labs when using other UARTs

  while (Serial.available())
    buffer[count++] = Serial.read();

  return count;
}

// Write to the serial port. Replaced later with
// bare-metal code

void writeSerial(const char *buffer, int len) {
  Serial.write(buffer, len);
  // Change Serial to Serial2/Serial3/Serial4 in later labs when using other
  // UARTs
}

void waitForHello() {
  int exit = 0;

  while (!exit) {
    TPacket hello;
    TResult result;

    do {
      result = readPacket(&hello);
    } while (result == PACKET_INCOMPLETE);

    if (result == PACKET_OK) {
      if (hello.packetType == PACKET_TYPE_HELLO) {
        sendOK();
        exit = 1;
      } else
        sendBadResponse();
    } else if (result == PACKET_BAD) {
      sendBadPacket();
    } else if (result == PACKET_CHECKSUM_BAD)
      sendBadChecksum();
  }  // !exit
}


void handleCommand(TPacket *command) {
  //Go directly to move
  sendOK();
  color_check();
  ultrasonicGetDistances();
  //sendStatus();
}

bool test = false;

ISR(SPI_STC_vect) {
  byte c = SPDR;
  if (test) {
    Serial.print("c = ");
    Serial.println((unsigned int)c);
  }

  switch (c) {
    case 1:
      SPDR = data[0];
      if (test) {
        Serial.print("L: ");
        Serial.print(data[0]);
        Serial.print(", ");
      }
      break;
    case 2:
      SPDR = data[1];
      if (test) {
        Serial.print("R: ");
        Serial.print(data[1]);
        Serial.print(", ");
      }
      break;
    case 3:
      SPDR = data[2];
      if (test) {
        Serial.print("F: ");
        Serial.print(data[2]);
        Serial.print(", ");
      }
      break;
    case 4:
      SPDR = data[3];
      if (test) {
        Serial.print("B: ");
        Serial.print(data[3]);
        Serial.print(", ");
      }
      break;
    case 5:
      SPDR = data[4];
      if (test) {
        Serial.print("Col: ");
        Serial.print(data[4]);
        Serial.println("");
      }
      break;
      /*
    case 6:
      SPDR = data[5];
      if (test) {
        Serial.print("Red: ");
        Serial.print(data[5]);
        Serial.println("");
      }
      break;
    case 7:
      SPDR = data[6];
      if (test) {
        Serial.print("Green: ");
        Serial.print(data[6]);
        Serial.println("");
      }
      break;
    case 8:
      SPDR = data[7];
      if (test) {
        Serial.print("Blue: ");
        Serial.print(data[7]);
        Serial.println("");
      }
      break;
      */
  }

}  // end of interrupt service routine (ISR) for SPI
