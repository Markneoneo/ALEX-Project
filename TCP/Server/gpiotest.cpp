#include <iostream>
#include <wiringPi.h>
#include <unistd.h>

using namespace std;

int main()
{
  if (wiringPiSetupGpio() == -1) {
      std::cerr << "Failed to init wiringPi lib.\n";
      return 1;
  }
  pinMode(1, INPUT);
  
  for (int i = 0; i < 26; i++) {
    pinMode(i, OUTPUT);
  }

  while(1) {
    cout << "Enter pin to test: ";
    int pinToTest;
    cin >> pinToTest;

        // Exit the program if the user enters -1
        if (pinToTest == -1) {
            break;
        }

        // Read the state of the pin
        digitalWrite(pinToTest, HIGH);
      std::cout << "Pin" << pinToTest << " is on.\n";
    usleep(200000);
    cout<<digitalRead(1)<<"\n";
    usleep(1000000);
    digitalWrite(pinToTest, LOW);
    std::cout << "Pin " << pinToTest << " is off.\n";
    usleep(200000);
    cout<<digitalRead(1) << "\n";
    usleep(1000000);
    
        
        /*
    digitalWrite(19, HIGH);
    std::cout << "Pin 19 is on.\n";
    usleep(1000000);
    digitalWrite(19, LOW);
    std::cout << "Pin 19 is off.\n";
    usleep(1000000);
    
    digitalWrite(21, HIGH);
    std::cout << "Pin 21 is on.\n";
    usleep(1000000);
    digitalWrite(21, LOW);
    std::cout << "Pin 21 is off.\n";
    usleep(1000000);
    
    digitalWrite(23, HIGH);
    std::cout << "Pin 23 is on.\n";
    usleep(1000000);
    digitalWrite(23, LOW);
    std::cout << "Pin 23 is off.\n";
    usleep(1000000);
    
    digitalWrite(25, HIGH);
    std::cout << "Pin 25 is on.\n";
    usleep(1000000);
    digitalWrite(25, LOW);
    std::cout << "Pin 25 is off.\n";
    usleep(1000000);
    */
  }

  return 0;
}
