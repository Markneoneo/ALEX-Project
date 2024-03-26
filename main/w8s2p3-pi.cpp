#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "serial.h"

typedef struct
{
	// Uncomment the line below for step 7 of activity 4.
	char c;
	int32_t x;
	int32_t y;
} TData;

// Replace with port name of the Arduino
#define PORT_NAME	"/dev/ttyACM0"

// Sleep time to wait for Arduino to boot up
#define SLEEP_TIME		2

int main()
{
	TData test;
	startSerial(PORT_NAME, B9600, 8, 'N', 1, 5);

	// The Arduino reboots when you connect to its
	// serial port. We sleep 2 seconds to wait for it
	// to finish rebooting.

	printf("Sleeping %d seconds to wait for Arduino to boot\n",
		SLEEP_TIME);
	sleep(SLEEP_TIME);

	while(1)
	{
		printf("Sending start character\n");
		char ch='s';
		serialWrite(&ch, sizeof(ch));
		// Now read in the data structure sent
		int len;
		char buffer[128];
		char tmpHold[128];
		int count=0;

		int targetSize=0;

		// Read port
		len = serialRead(buffer);

		// First byte is the size
		targetSize=buffer[0];

		printf("Size of TData on this machine is %d bytes\n", sizeof(TData));
		printf("Size of TData on the Arduino is %d bytes\n", targetSize);

		int i;
		for(i=1; i<len; i++)
			tmpHold[count++] = buffer[i];

		while(count<targetSize)
		{
			len = serialRead(buffer);
			if(len > 0)
			{
				memcpy(&tmpHold[count], buffer, len);
				count+=len;
			}
		}

		// Now we convert to our data structure
		memcpy(&test, tmpHold, sizeof(TData));

		printf("x received is %d\n", test.x);
		printf("y received is %d\n", test.y);

		// Uncomment the line below for Step 7 of Activity 4 
		// in Week 8 Studio 2.
		printf("c received is %c\n", test.c);
		sleep(1);
	}

}
