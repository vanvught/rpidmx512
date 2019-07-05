#if !defined(ORANGE_PI)
/**
 * @file slushtemp.cpp
 *
 */
/*
 * Based on https://github.com/Roboteurs/slushengine/blob/master/Slush/Temprature.py
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <math.h>

#include "hal_i2c.h"

#include "slushboard.h"

#define MAX1164_I2C_ADDRESS		0x36

#define POTENTIAL_DIVIDER_RESISTOR	100000
#define THERMISTOR_B_VALUE			3950
#define THERMISTOR_REF_TEMP			298.15
#define THERMISTOR_REF_RESISTANCE	50000

uint16_t SlushBoard::getTempRaw(void) {
	uint8_t buf[2] = { 0, 0 };

	FUNC_PREFIX(i2c_set_address(MAX1164_I2C_ADDRESS));
	FUNC_PREFIX(i2c_set_baudrate(I2C_FULL_SPEED));
	FUNC_PREFIX(i2c_read((char *) buf, (uint32_t) 2));

	return (uint16_t) ((uint16_t) buf[0] << 8 | (uint16_t) buf[1]);
}

float SlushBoard::getTemprature(void) {
	return calcTemp(getTempRaw());
}

float SlushBoard::calcTemp(uint16_t tempraw) {
	const float voltage = (float) tempraw / 1024 * 5;
	const float resistance = (float) POTENTIAL_DIVIDER_RESISTOR / (5 / voltage - 1);
	const float temp = (float) 1 / (1 / THERMISTOR_REF_TEMP + logf(resistance / THERMISTOR_REF_RESISTANCE) / THERMISTOR_B_VALUE);

	return temp - 273.15;
}
#endif
