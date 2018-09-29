/**
 * @file pca9685dmxservo.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef PWMDMXPCA9685SERVO_H_
#define PWMDMXPCA9685SERVO_H_

#include <stdint.h>

#include "lightset.h"

#include "pca9685servo.h"

class PCA9685DmxServo: public LightSet {
public:
	PCA9685DmxServo(void);
	~PCA9685DmxServo(void);

	bool SetDmxStartAddress(uint16_t nDmxStartAddress);

	inline uint16_t GetDmxStartAddress(void) {
		return m_nDmxStartAddress;
	}

	inline uint16_t GetDmxFootprint(void) {
		return m_nDmxFootprint;
	}

	void Start(uint8_t nPort = 0);
	void Stop(uint8_t nPort = 0);

	void SetData(uint8_t nPort, const uint8_t *pDmxData, uint16_t nLength);

public:
	void SetI2cAddress(uint8_t nI2cAddress);
	void SetBoardInstances(uint8_t nBoardInstances);
	void SetLeftUs(uint16_t nLeftUs);
	void SetRightUs(uint16_t nRightUs);

	void SetDmxFootprint(uint16_t nDmxFootprint);

private:
	void Initialize(void);

private:
	uint16_t m_nDmxStartAddress;
	uint16_t m_nDmxFootprint;
	uint8_t m_nI2cAddress;
	uint8_t m_nBoardInstances;
	uint16_t m_nLeftUs;
	uint16_t m_nRightUs;
	bool m_bIsStarted;
	PCA9685Servo **m_pServo;
	uint8_t *m_pDmxData;
};

#endif /* PWMDMXPCA9685SERVO_H_ */
