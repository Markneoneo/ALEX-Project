#include <iostream>
#include <wiringPiSPI.h>
#include <chrono>
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
using namespace std;
#define SPI_CHANNEL 0
#define SPI_CLOCK_SPEED 1000000


int main(int argc, char **argv)
{
  int fd = wiringPiSPISetupMode(SPI_CHANNEL, SPI_CLOCK_SPEED, 0);
  if (fd == -1) {
      std::cout << "Failed to init SPI communication.\n";
      return -1;
  }
  std::cout << "SPI communication successfully setup.\n";
  int data = 0;

  //Start TCP Server on port
  //int serverSocket = socket(AF_INET,SOCK_STREAM,0);
  //int PORT = 55552;
  //
  ////Set sockopt
  //int opt = 1;
  //setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));


  ////Defining the address
  //sockaddr_in serverAddress;
  //sockaddr_in clientAddress;
  //memset(&clientAddress, 0, sizeof(clientAddress));
  //serverAddress.sin_family = AF_INET;
  //serverAddress.sin_port = htons(PORT);
  //serverAddress.sin_addr.s_addr = INADDR_ANY; //Listen from all IP addresses

  ////Bind the server socket to the server
  //bind(serverSocket,(struct sockaddr*)&serverAddress,sizeof(serverAddress));

  ////Listen for connection
  //if(listen(serverSocket,10) == -1) {
  //  printf("Error: Failed to listen\n");
  //  close(serverSocket);
  //  return 1;
  //}

  //printf("Server is listening on port %d\n",PORT);

  ////Accept incoming connection
  //socklen_t client_address_len = sizeof(clientAddress);
  //int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &client_address_len);
  //while(clientSocket == -1) {
  //  printf("Failed to connect\n");
  //  clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &client_address_len);
  //}

  //printf("Successfully connected\n");
  //char buffer[12] = {0};

  while(1) {
    //Wait for send signal from client
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    //while(1) {
    //  int bytesReceived = ::recv(clientSocket,buffer,sizeof(buffer),0);
    //  if(bytesReceived != -1) {
    //    //Check is buffer contains send
    //    printf("DATA RECEIVED!!");
    //    if(strstr(buffer,"receive")) break;
    //  }
    //}

    string message;
    while(1) {
      unsigned char buf[2] = { 0, 0 };
      wiringPiSPIDataRW(SPI_CHANNEL, buf, 2);
      data = buf[1];
      message = std::to_string(data);
      message = message + ",";
      //std:: cout <<"DATA 0: "<<data<<std::endl;

      unsigned char buf2[2] = { 1, 0 };
      wiringPiSPIDataRW(SPI_CHANNEL, buf2, 2);
      data = buf2[1];
      message = message + std::to_string(data) + ",";
      //std:: cout <<"DATA 1: "<<data<<std::endl;

      unsigned char buf3[2] = { 2, 0 };
      wiringPiSPIDataRW(SPI_CHANNEL, buf3, 2);
      data = buf3[1];
      message = message + std::to_string(data) + ",";
      //std:: cout <<"DATA 2: "<<data<<std::endl;

      unsigned char buf4[2] = { 3, 0 };
      wiringPiSPIDataRW(SPI_CHANNEL, buf4, 2);
      data = buf4[1];
      message = message + std::to_string(data);
      //std:: cout <<"DATA 3: "<<data<<std::endl;

      //Send data to client
      //send(clientSocket,message.c_str(),strlen(message.c_str()),0);
      std::cout << message <<std::endl;

      std::this_thread::sleep_for(std::chrono::milliseconds(600));
    }
  }

   
  return 0;
}
