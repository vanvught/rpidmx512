/**
 * https://github.com/Roboteurs/slushengine/blob/master/examples/precise_move.py
 */
/*
#initalizes the board and all its functions
SlushEngine = Slush.sBoard()

#initalizes the motor on the board
Motor = Slush.Motor(0)
Motor.resetDev()
Motor.setCurrent(20, 20, 20, 20)

#wait for the move to be not busy
while(Motor.isBusy()):
	continue

#move the motor at a new microstepping setting
print("-> moving the motor at 128 Step/Step microstepping")
Motor.setMicroSteps(128)
Motor.move(-100000)
while(Motor.isBusy()):
	continue
print("-> motor is free move it as you like, you have 5 seconds!")
Motor.free()
time.sleep(5)

#set the current position to home using the drivers internal logic
print("-> setting the current position to home")
Motor.setAsHome()
time.sleep(1)

#moving to a position with refrence to home
print("-> moving 200000 steps from home position")
Motor.goTo(200000)
while(Motor.isBusy()):
	continue
time.sleep(1)

#moving to a position with refrence to home
print("-> moving -300000 steps from home position")
Motor.goTo(-300000)
while(Motor.isBusy()):
	continue
time.sleep(1)

#run the motor for a short period of time
print("-> running the motor to random position")
Motor.run(1, 100)
time.sleep(5) #can be changed to any value

#return the motor to home
print("-> returning the motor to home position")
Motor.goHome()

#free the motor
while(Motor.isBusy()):
	continue
Motor.free()
*/

#include <stdio.h>
#include <unistd.h>

#include "slushboard.h"
#include "slushmotor.h"

int main(int argc, char **argv) {
	SlushBoard SlushEngine;
	SlushMotor Motor(0);
	//SlushMotor Motor(0, false); // Use /BUSY pin instead of SPI STATUS register

	Motor.resetDev();
	Motor.setCurrent(20, 20, 20, 20);

	while (Motor.isBusy())
		;

// move the motor at a new microstepping setting
	printf("-> moving the motor at 128 Step/Step microstepping\n");
	Motor.setMicroSteps(128);
	Motor.move(-100000);
	while (Motor.isBusy())
		;

	printf("-> motor is free move it as you like, you have 5 seconds!\n");
	Motor.free();

	sleep(5);

// set the current position to home using the drivers internal logic
	printf("-> setting the current position to home\n");
	Motor.setAsHome();
	sleep(1);

// moving to a position with refrence to home
	printf("-> moving 200000 steps from home position\n");
	Motor.goTo(200000);
	while (Motor.isBusy())
		;

	sleep(1);

// moving to a position with refrence to home
	printf("-> moving -300000 steps from home position\n");
	Motor.goTo(-300000);
	while (Motor.isBusy())
		;

	sleep(1);

// run the motor for a short period of time
	printf("-> running the motor to random position\n");
	Motor.run(1, 100);
	sleep(5);	// can be changed to any value

// #return the motor to home
	printf("-> returning the motor to home position\n");
	Motor.goHome();

	while (Motor.isBusy())
		;

	Motor.free();

	return 0;
}
