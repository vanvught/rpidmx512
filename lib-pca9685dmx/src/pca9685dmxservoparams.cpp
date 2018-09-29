/**
 * @file pca9685dmxservoparams.cpp
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

#include <stdint.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__((aligned(4)))
#endif

#include "pca9685dmxservoparams.h"
#include "pca9685servo.h"

#include "readconfigfile.h"
#include "sscan.h"

#define LEFT_US_MASK			(1 << 0)
#define RIGHT_US_MASK			(1 << 1)
#define I2C_SLAVE_ADDRESS_MASK	(1 << 2)

static const char PARAMS_FILE_NAME[] ALIGNED = "servo.txt";
static const char PARAMS_I2C_SLAVE_ADDRESS[] ALIGNED = "i2c_slave_address";
static const char PARAMS_LEFT_US[] ALIGNED = "left_us";
static const char PARAMS_RIGHT_US[] ALIGNED = "right_us";

PCA9685DmxServoParams::PCA9685DmxServoParams(void) :
	PCA9685DmxParams(PARAMS_FILE_NAME),
	m_bSetList(0),
	m_nI2cAddress(PCA9685_I2C_ADDRESS_DEFAULT),
	m_nLeftUs(SERVO_LEFT_DEFAULT_US), m_nRightUs(SERVO_RIGHT_DEFAULT_US)
{
}

PCA9685DmxServoParams::~PCA9685DmxServoParams(void) {
}

bool PCA9685DmxServoParams::Load(void) {
	ReadConfigFile configfile(PCA9685DmxServoParams::staticCallbackFunction, this);
	return configfile.Read(PARAMS_FILE_NAME);
}

void PCA9685DmxServoParams::Set(PCA9685DmxServo* pDmxServo) {
	assert(pDmxServo != 0);

	bool isSet;

	if ((!GetSetList()) && (m_bSetList == 0)) {
		return;
	}

	if(isMaskSet(I2C_SLAVE_ADDRESS_MASK)) {
		pDmxServo->SetI2cAddress(m_nI2cAddress);
	}

	if(isMaskSet(LEFT_US_MASK)) {
		pDmxServo->SetLeftUs(m_nLeftUs);
	}

	if(isMaskSet(RIGHT_US_MASK)) {
		pDmxServo->SetRightUs(m_nRightUs);
	}

	const uint16_t DmxStartAddress = GetDmxStartAddress(isSet);
	if (isSet) {
		pDmxServo->SetDmxStartAddress(DmxStartAddress);
	}

	const uint8_t BoardInstances = GetBoardInstances(isSet);
	if (isSet) {
		pDmxServo->SetBoardInstances(BoardInstances);
	}

	// Footprint overwrites board instances!
	const uint16_t DmxFootprint = GetDmxFootprint(isSet);
	if (isSet) {
		pDmxServo->SetDmxFootprint(DmxFootprint);
	}
}

void PCA9685DmxServoParams::Dump(void) {
#ifndef NDEBUG
	if ((!GetSetList()) && (m_bSetList == 0)) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__,__FUNCTION__, PARAMS_FILE_NAME);

	if(isMaskSet(I2C_SLAVE_ADDRESS_MASK)) {
		printf(" %s=0x%2x\n", PARAMS_I2C_SLAVE_ADDRESS, m_nI2cAddress);
	}

	if(isMaskSet(LEFT_US_MASK)) {
		printf(" %s=%d\n", PARAMS_LEFT_US, m_nLeftUs);
	}

	if(isMaskSet(RIGHT_US_MASK)) {
		printf(" %s=%d\n", PARAMS_RIGHT_US, m_nRightUs);
	}

	PCA9685DmxParams::Dump();
#endif
}

bool PCA9685DmxServoParams::isMaskSet(uint32_t nMask) const {
	return (m_bSetList & nMask) == nMask;
}

void PCA9685DmxServoParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((PCA9685DmxServoParams *) p)->callbackFunction(s);
}

void PCA9685DmxServoParams::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint16_t value16;

	if (Sscan::I2cAddress(pLine, PARAMS_I2C_SLAVE_ADDRESS, &value8) == SSCAN_OK) {
		if ((value8 >= PCA9685_I2C_ADDRESS_DEFAULT) && (value8 != PCA9685_I2C_ADDRESS_FIXED)) {
			m_nI2cAddress = value8;
			m_bSetList |= I2C_SLAVE_ADDRESS_MASK;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_LEFT_US, &value16) == SSCAN_OK) {
		if ((value16 != 0) && (value16 < m_nRightUs)) {
			m_nLeftUs = value16;
			m_bSetList |= LEFT_US_MASK;
		}
		return;
	}

	if (Sscan::Uint16(pLine, PARAMS_RIGHT_US, &value16) == SSCAN_OK) {
		if (value16 > m_nLeftUs) {
			m_nRightUs = value16;
			m_bSetList |= RIGHT_US_MASK;
		}
		return;
	}
}
