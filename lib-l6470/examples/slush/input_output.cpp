/**
 * https://github.com/Roboteurs/slushengine/blob/master/examples/input_output.py
 */
/*
#initalizes the board and all its functions
SlushEngine = Slush.sBoard()

while(1):

	#reads pin A0 and prints the value
	print("Pin A0: " + str(SlushEngine.getIOState(0, 0)))

	#reads the value of pin B0 and sets an LED based on the reading connected to A7
	if SlushEngine.getIOState(1, 0) == 1:
		SlushEngine.setIOState(0, 7, 1)
	else:
		SlushEngine.setIOState(0, 7, 0)

	time.sleep(1)
*/

#include <stdio.h>
#include <unistd.h>

#include "slushboard.h"

int main(int argc, char **argv) {
	SlushBoard SlushEngine;

	while(1) {

		// reads pin A0 and prints the value
		printf("Pin A0: %d\n", SlushEngine.getIOState(0, 0));

		printf("\tPort B: %x\n", SlushEngine.IORead(SLUSH_IO_PORTB));

		//reads the value of pin B0 and sets an LED based on the reading connected to A7
		if (SlushEngine.getIOState(1, 0) == 1)
			SlushEngine.setIOState(0, 7, 1);
		else
			SlushEngine.setIOState(0, 7, 0);

		sleep(1);

	}
}
