/**
 * @file servo.h
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

#ifndef PCA9685SERVO_H_
#define PCA9685SERVO_H_

#include <stdint.h>

#include "pca9685.h"

#define ANGLE(x)	((uint8_t)(x))

#define SERVO_LEFT_DEFAULT_US	1000
#define SERVO_CENTER_DEFAULT_US	1500
#define SERVO_RIGHT_DEFAULT_US 	2000

class PCA9685Servo: public PCA9685 {
public:
	PCA9685Servo(uint8_t nAddress = 0x40);
	~PCA9685Servo(void);

	void SetLeftUs(uint16_t);
	uint16_t GetLeftUs(void) const;

	void SetRightUs(uint16_t);
	uint16_t GetRightUs(void) const;

	void Set(uint8_t nChannel, uint16_t nData);
	void Set(uint8_t nChannel, uint8_t nData);

	void SetAngle(uint8_t nChannel, uint8_t nAngle);

private:
	void CalcLeftCount(void);
	void CalcRightCount(void);

private:
	uint16_t m_nLeftUs;
	uint16_t m_nRightUs;
	uint16_t m_nLeftCount;
	uint16_t m_nRightCount;
};

#endif /* PCA9685SERVO_H_ */
