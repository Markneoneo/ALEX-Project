#include <iostream>
#include <wiringPiSPI.h>
#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <ostream>
#include <stdlib.h>
#include <fcntl.h>
#include <string>

#define SPI_CHANNEL 0
#define SPI_CLOCK_SPEED 100000
using namespace std;

int main(int argc, char **argv)
{
  int fd = wiringPiSPISetupMode(SPI_CHANNEL, SPI_CLOCK_SPEED, 0);
  if (fd == -1) {
      std::cout << "Failed to init SPI communication.\n";
      return -1;
  }
  std::cout << "SPI communication successfully setup.\n";
  int data = 0;

  usleep(1000000); // Wait 1 second

  while(1) {
    
      string empty = "";
      string message = "";
      
      unsigned char buf[2] = { 1 , 0 };
      wiringPiSPIDataRW(SPI_CHANNEL, buf, 2);
      data = buf[1];
      message += "Left:  " + std::to_string(data);
      message = message + ",";

      unsigned char buf2[2] = { 2 , 0 };
      wiringPiSPIDataRW(SPI_CHANNEL, buf2, 2);
      data = buf2[1];
      message += "Right: " + std::to_string(data);
      message = message + ",";
      
      unsigned char buf3[2] = { 3 , 0 };
      wiringPiSPIDataRW(SPI_CHANNEL, buf3, 2);
      data = buf3[1];
      message += "Front: " + std::to_string(data);
      message = message + ",";
      
      unsigned char buf4[2] = { 4 , 0 };
      wiringPiSPIDataRW(SPI_CHANNEL, buf4, 2);
      data = buf4[1];
      message += "Back: " + std::to_string(data) + ",";

      unsigned char buf5[2] = { 5 , 0 };
      wiringPiSPIDataRW(SPI_CHANNEL, buf5, 2);
      data = buf5[1];
      message += "Color: " + std::to_string(data);
     
      //Send data to client
      std::cout << message <<std::endl;

      usleep(1000000); // Sleep 2 seconds
  }


  return 0;
}
