/**
 * @file l6470support.cpp
 *
 */
/*
 * Based on https://github.com/sparkfun/L6470-AutoDriver/tree/master/Libraries/Arduino
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdint.h>
#include <stdbool.h>
#ifndef NDEBUG
#include <stdio.h>
#endif

#include "l6470.h"

static unsigned long ceil(float f) {
	int i = (int) f;
	if (f == (float) i) {
		return i;
	}
	return i + 1;
}

// Contains utility functions for converting real-world
//  units (eg, steps/s) to values usable by the dsPIN controller. These are all
//  private members of class AutoDriver.

// The value in the ACC register is [(steps/s/s)*(tick^2)]/(2^-40) where tick is 
//  250ns (datasheet value)- 0x08A on boot.
// Multiply desired steps/s/s by .137438 to get an appropriate value for this register.
// This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
unsigned long L6470::accCalc(float stepsPerSecPerSec) {
	float temp = stepsPerSecPerSec * 0.137438;
	if ((unsigned long) long(temp) > 0x00000FFF)
		return 0x00000FFF;
	else
		return (unsigned long) long(temp);
}

float L6470::accParse(unsigned long stepsPerSecPerSec) {
	return (float) (stepsPerSecPerSec & 0x00000FFF) / 0.137438;
}

// The calculation for DEC is the same as for ACC. Value is 0x08A on boot.
// This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
unsigned long L6470::decCalc(float stepsPerSecPerSec) {
	float temp = stepsPerSecPerSec * 0.137438;
	if ((unsigned long) long(temp) > 0x00000FFF)
		return 0x00000FFF;
	else
		return (unsigned long) long(temp);
}

float L6470::decParse(unsigned long stepsPerSecPerSec) {
	return (float) (stepsPerSecPerSec & 0x00000FFF) / 0.137438;
}

// The value in the MAX_SPD register is [(steps/s)*(tick)]/(2^-18) where tick is 
//  250ns (datasheet value)- 0x041 on boot.
// Multiply desired steps/s by .065536 to get an appropriate value for this register
// This is a 10-bit value, so we need to make sure it remains at or below 0x3FF
unsigned long L6470::maxSpdCalc(float stepsPerSec) {
	unsigned long temp = ceil(stepsPerSec * .065536);
	if (temp > 0x000003FF)
		return 0x000003FF;
	else
		return temp;
}

float L6470::maxSpdParse(unsigned long stepsPerSec) {
	return (float) (stepsPerSec & 0x000003FF) / 0.065536;
}

// The value in the MIN_SPD register is [(steps/s)*(tick)]/(2^-24) where tick is 
//  250ns (datasheet value)- 0x000 on boot.
// Multiply desired steps/s by 4.1943 to get an appropriate value for this register
// This is a 12-bit value, so we need to make sure the value is at or below 0xFFF.
unsigned long L6470::minSpdCalc(float stepsPerSec) {
	float temp = stepsPerSec / 0.238;
	if ((unsigned long) long(temp) > 0x00000FFF)
		return 0x00000FFF;
	else
		return (unsigned long) long(temp);
}

float L6470::minSpdParse(unsigned long stepsPerSec) {
	return (float) ((stepsPerSec & 0x00000FFF) * 0.238);
}

// The value in the FS_SPD register is ([(steps/s)*(tick)]/(2^-18))-0.5 where tick is 
//  250ns (datasheet value)- 0x027 on boot.
// Multiply desired steps/s by .065536 and subtract .5 to get an appropriate value for this register
// This is a 10-bit value, so we need to make sure the value is at or below 0x3FF.
unsigned long L6470::FSCalc(float stepsPerSec) {
	float temp = (stepsPerSec * .065536) - .5;
	if ((unsigned long) long(temp) > 0x000003FF)
		return 0x000003FF;
	else
		return (unsigned long) long(temp);
}

float L6470::FSParse(unsigned long stepsPerSec) {
	return (((float) (stepsPerSec & 0x000003FF)) + 0.5) / 0.065536;
}

// The value in the INT_SPD register is [(steps/s)*(tick)]/(2^-24) where tick is 
//  250ns (datasheet value)- 0x408 on boot.
// Multiply desired steps/s by 4.1943 to get an appropriate value for this register
// This is a 14-bit value, so we need to make sure the value is at or below 0x3FFF.
unsigned long L6470::intSpdCalc(float stepsPerSec) {
	float temp = stepsPerSec * 4.1943;
	if ((unsigned long) long(temp) > 0x00003FFF)
		return 0x00003FFF;
	else
		return (unsigned long) long(temp);
}

float L6470::intSpdParse(unsigned long stepsPerSec) {
	return (float) (stepsPerSec & 0x00003FFF) / 4.1943;
}

// When issuing RUN command, the 20-bit speed is [(steps/s)*(tick)]/(2^-28) where tick is 
//  250ns (datasheet value).
// Multiply desired steps/s by 67.106 to get an appropriate value for this register
// This is a 20-bit value, so we need to make sure the value is at or below 0xFFFFF.
unsigned long L6470::spdCalc(float stepsPerSec) {
	unsigned long temp = stepsPerSec * 67.106;
	if (temp > 0x000FFFFF)
		return 0x000FFFFF;
	else
		return temp;
}

float L6470::spdParse(unsigned long stepsPerSec) {
	return (float) (stepsPerSec & 0x000FFFFF) / 67.106;
}

// Much of the functionality between "get parameter" and "set parameter" is
//  very similar, so we deal with that by putting all of it in one function
//  here to save memory space and simplify the program.
long L6470::paramHandler(uint8_t param, unsigned long value) {
	long retVal = 0;   // This is a temp for the value to return.

	// This switch structure handles the appropriate action for each register.
	//  This is necessary since not all registers are of the same length, either
	//  bit-wise or uint8_t-wise, so we want to make sure we mask out any spurious
	//  bits and do the right number of transfers. That is handled by the xferParam()
	//  function, in most cases, but for 1-uint8_t or smaller transfers, we call
	//  SPIXfer() directly.
	switch (param) {
	// ABS_POS is the current absolute offset from home. It is a 22 bit number expressed
	//  in two's complement. At power up, this value is 0. It cannot be written when
	//  the motor is running, but at any other time, it can be updated to change the
	//  interpreted position of the motor.
	case L6470_PARAM_ABS_POS:
		retVal = xferParam(value, 22);
		break;
		// EL_POS is the current electrical position in the step generation cycle. It can
		//  be set when the motor is not in motion. Value is 0 on power up.
	case L6470_PARAM_EL_POS:
		retVal = xferParam(value, 9);
		break;
		// MARK is a second position other than 0 that the motor can be told to go to. As
		//  with ABS_POS, it is 22-bit two's complement. Value is 0 on power up.
	case L6470_PARAM_MARK:
		retVal = xferParam(value, 22);
		break;
		// SPEED contains information about the current speed. It is read-only. It does
		//  NOT provide direction information.
	case L6470_PARAM_SPEED:
		retVal = xferParam(0, 20);
		break;
		// ACC and DEC set the acceleration and deceleration rates. Set ACC to 0xFFF
		//  to get infinite acceleration/decelaeration- there is no way to get infinite
		//  deceleration w/o infinite acceleration (except the HARD STOP command).
		//  Cannot be written while motor is running. Both default to 0x08A on power up.
		// AccCalc() and DecCalc() functions exist to convert steps/s/s values into
		//  12-bit values for these two registers.
	case L6470_PARAM_ACC:
		retVal = xferParam(value, 12);
		break;
	case L6470_PARAM_DECEL:
		retVal = xferParam(value, 12);
		break;
		// MAX_SPEED is just what it says- any command which attempts to set the speed
		//  of the motor above this value will simply cause the motor to turn at this
		//  speed. Value is 0x041 on power up.
		// MaxSpdCalc() function exists to convert steps/s value into a 10-bit value
		//  for this register.
	case L6470_PARAM_MAX_SPEED:
		retVal = xferParam(value, 10);
		break;
		// MIN_SPEED controls two things- the activation of the low-speed optimization
		//  feature and the lowest speed the motor will be allowed to operate at. LSPD_OPT
		//  is the 13th bit, and when it is set, the minimum allowed speed is automatically
		//  set to zero. This value is 0 on startup.
		// MinSpdCalc() function exists to convert steps/s value into a 12-bit value for this
		//  register. SetLSPDOpt() function exists to enable/disable the optimization feature.
	case L6470_PARAM_MIN_SPEED:
		retVal = xferParam(value, 13);
		break;
		// FS_SPD register contains a threshold value above which microstepping is disabled
		//  and the dSPIN operates in full-step mode. Defaults to 0x027 on power up.
		// FSCalc() function exists to convert steps/s value into 10-bit integer for this
		//  register.
	case L6470_PARAM_FS_SPD:
		retVal = xferParam(value, 10);
		break;
		// KVAL is the maximum voltage of the PWM outputs. These 8-bit values are ratiometric
		//  representations: 255 for full output voltage, 128 for half, etc. Default is 0x29.
		// The implications of different KVAL settings is too complex to dig into here, but
		//  it will usually work to max the value for RUN, ACC, and DEC. Maxing the value for
		//  HOLD may result in excessive power dissipation when the motor is not running.
	case L6470_PARAM_KVAL_HOLD:
		retVal = xferParam(value, 8);
		break;
	case L6470_PARAM_KVAL_RUN:
		retVal = xferParam(value, 8);
		break;
	case L6470_PARAM_KVAL_ACC:
		retVal = xferParam(value, 8);
		break;
	case L6470_PARAM_KVAL_DEC:
		retVal = xferParam(value, 8);
		break;
		// INT_SPD, ST_SLP, FN_SLP_ACC and FN_SLP_DEC are all related to the back EMF
		//  compensation functionality. Please see the datasheet for details of this
		//  function- it is too complex to discuss here. Default values seem to work
		//  well enough.
	case L6470_PARAM_INT_SPD:
		retVal = xferParam(value, 14);
		break;
	case L6470_PARAM_ST_SLP:
		retVal = xferParam(value, 8);
		break;
	case L6470_PARAM_FN_SLP_ACC:
		retVal = xferParam(value, 8);
		break;
	case L6470_PARAM_FN_SLP_DEC:
		retVal = xferParam(value, 8);
		break;
		// K_THERM is motor winding thermal drift compensation. Please see the datasheet
		//  for full details on operation- the default value should be okay for most users.
	case L6470_PARAM_K_THERM:
		value &= 0x0F;
		retVal = xferParam(value, 8);
		break;
		// ADC_OUT is a read-only register containing the result of the ADC measurements.
		//  This is less useful than it sounds; see the datasheet for more information.
	case L6470_PARAM_ADC_OUT:
		retVal = xferParam(value, 8);
		break;
		// Set the overcurrent threshold. Ranges from 375mA to 6A in steps of 375mA.
		//  A set of defined constants is provided for the user's convenience. Default
		//  value is 3.375A- 0x08. This is a 4-bit value.
	case L6470_PARAM_OCD_TH:
		value &= 0x0F;
		retVal = xferParam(value, 8);
		break;
		// Stall current threshold. Defaults to 0x40, or 2.03A. Value is from 31.25mA to
		//  4A in 31.25mA steps. This is a 7-bit value.
	case L6470_PARAM_STALL_TH:
		value &= 0x7F;
		retVal = xferParam(value, 8);
		break;
		// STEP_MODE controls the microstepping settings, as well as the generation of an
		//  output signal from the dSPIN. Bits 2:0 control the number of microsteps per
		//  step the part will generate. Bit 7 controls whether the BUSY/SYNC pin outputs
		//  a BUSY signal or a step synchronization signal. Bits 6:4 control the frequency
		//  of the output signal relative to the full-step frequency; see datasheet for
		//  that relationship as it is too complex to reproduce here.
		// Most likely, only the microsteps per step value will be needed; there is a set
		//  of constants provided for ease of use of these values.
	case L6470_PARAM_STEP_MODE:
		retVal = xferParam(value, 8);
		break;
		// ALARM_EN controls which alarms will cause the FLAG pin to fall. A set of constants
		//  is provided to make this easy to interpret. By default, ALL alarms will trigger the
		//  FLAG pin.
	case L6470_PARAM_ALARM_EN:
		retVal = xferParam(value, 8);
		break;
		// CONFIG contains some assorted configuration bits and fields. A fairly comprehensive
		//  set of reasonably self-explanatory constants is provided, but users should refer
		//  to the datasheet before modifying the contents of this register to be certain they
		//  understand the implications of their modifications. Value on boot is 0x2E88; this
		//  can be a useful way to verify proper start up and operation of the dSPIN chip.
	case L6470_PARAM_CONFIG:
		retVal = xferParam(value, 16);
		break;
		// STATUS contains read-only information about the current condition of the chip. A
		//  comprehensive set of constants for masking and testing this register is provided, but
		//  users should refer to the datasheet to ensure that they fully understand each one of
		//  the bits in the register.
	case L6470_PARAM_STATUS:  // STATUS is a read-only register
		retVal = xferParam(0, 16);
		break;
	default:
		SPIXfer((uint8_t) value);
		break;
	}
	return retVal;
}

typedef union {
	unsigned long ul;
	uint8_t a[4];
} _cast;

long L6470::xferParam(unsigned long nValue, uint8_t nBitLength) {
#ifndef NDEBUG
	printf("%s value = %ld, bitLen = %d\n", __FUNCTION__, nValue, (int) nBitLength);
#endif
	_cast in;
	_cast out;

	uint8_t byteLen = nBitLength / 8;

	if (nBitLength % 8 > 0) {
		byteLen++;
	}

	in.ul = nValue;
	out.ul = 0;

	for (int i = 0; i < byteLen; i++) {
		out.a[byteLen - i - 1] = SPIXfer(in.a[byteLen - i - 1]);
	}

	return out.ul;
}
