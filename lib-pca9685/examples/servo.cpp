/**
 * @file servo.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "pca9685servo.h"

int main(int argc, char **argv) {
	if (getuid() != 0) {
		fprintf(stderr, "Program is not started as \'root\' (sudo)\n");
		return -1;
	}

	PCA9685Servo servo;

	// MG90S Micro Servo
	servo.SetLeftUs(700);
	servo.SetRightUs(2400);

	servo.Dump();

	puts("");

	servo.SetAngle(CHANNEL(0), ANGLE(90));
	servo.SetAngle(CHANNEL(1), ANGLE(90));
	servo.SetAngle(CHANNEL(2), ANGLE(90));
	servo.SetAngle(CHANNEL(3), ANGLE(90));

	puts("Servo 0:90  Servo 1:90");
	puts("Servo 2:90  Servo 3:90");

	sleep(4);

	for (;;) {
		servo.SetAngle(CHANNEL(0), ANGLE(90));
		servo.SetAngle(CHANNEL(1), ANGLE(0));
		servo.SetAngle(CHANNEL(2), ANGLE(0));
		servo.SetAngle(CHANNEL(3), ANGLE(0));
		puts("");
		puts("Servo 0:90  Servo 1:0");
		puts("Servo 2:0  Servo 3:0");
		sleep(2);
		servo.SetAngle(CHANNEL(0), ANGLE(180));
		servo.SetAngle(CHANNEL(1), ANGLE(180));
		servo.SetAngle(CHANNEL(2), ANGLE(90));
		servo.SetAngle(CHANNEL(3), ANGLE(90));
		puts("");
		puts("Servo 0:180 Servo 1:180");
		puts("Servo 2:90  Servo 3:90");
		sleep(2);
		servo.SetAngle(CHANNEL(0), ANGLE(0));
		servo.SetAngle(CHANNEL(1), ANGLE(90));
		servo.SetAngle(CHANNEL(2), ANGLE(180));
		servo.SetAngle(CHANNEL(3), ANGLE(180));
		puts("");
		puts("Servo 0:0   Servo 1:90");
		puts("Servo 2:180  Servo 3:180");
		sleep(2);
	}

	return 0;
}
