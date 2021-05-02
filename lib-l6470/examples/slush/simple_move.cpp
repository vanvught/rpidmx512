/**
 * https://github.com/Roboteurs/slushengine/blob/master/examples/simple_move.py
 */
/*
#the number of steps we want to be moving
stepmove = 300000

#setup the Slushengine
b = Slush.sBoard()
axis1 = Slush.Motor(0)
axis1.resetDev()
axis1.setCurrent(50, 50, 50, 50)

#move the motor in one direction and wait for it to finish
while(axis1.isBusy()):
	continue
axis1.move(stepmove)

while(axis1.isBusy()):
	continue
axis1.move(-1 * stepmove)

#when these operations are finished shut off the motor
while(axis1.isBusy()):
	continue
axis1.free()
*/

#include <stdio.h>

#include "slushboard.h"
#include "slushmotor.h"

int main(int argc, char **argv) {
	SlushBoard SlushEngine;
	SlushMotor axis1(0);
	//SlushMotor axis1(0, false); // Use /BUSY pin instead of SPI STATUS register

	const int stepmove = 300000;// #the number of steps we want to be moving

	axis1.resetDev();

	axis1.setCurrent(50, 50, 50, 50);

	while (axis1.isBusy())
		;		// #move the motor in one direction and wait for it to finish

	axis1.move(stepmove);

	while (axis1.isBusy())
		;

	axis1.move(-stepmove);

	while (axis1.isBusy())
		;	// #when these operations are finished shut off the motor

	axis1.free();

	return 0;
}
