/**
 * @file pca9685dmxservo.cpp
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

#include "pca9685dmxservo.h"

#define DMX_MAX_CHANNELS	512
#define BOARD_INSTANCES_MAX	32

static unsigned long ceil(float f) {
	int i = (int) f;
	if (f == (float) i) {
		return i;
	}
	return i + 1;
}

PCA9685DmxServo::PCA9685DmxServo(void):
	m_nDmxStartAddress(1),
	m_nDmxFootprint(PCA9685_PWM_CHANNELS),
	m_nI2cAddress(PCA9685_I2C_ADDRESS_DEFAULT),
	m_nBoardInstances(1),
	m_nLeftUs(SERVO_LEFT_DEFAULT_US),
	m_nRightUs(SERVO_RIGHT_DEFAULT_US),
	m_bIsStarted(false),
	m_pServo(0),
	m_pDmxData(0)
{
}

PCA9685DmxServo::~PCA9685DmxServo(void) {
	delete m_pServo;
	m_pServo = 0;
}

bool PCA9685DmxServo::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= DMX_MAX_CHANNELS));

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= DMX_MAX_CHANNELS)) {
		m_nDmxStartAddress = nDmxStartAddress;
		return true;
	}

	return false;
}

void PCA9685DmxServo::Start(uint8_t nPort) {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	if (__builtin_expect((m_pServo == 0), 0)) {
		Initialize();
	}
}

void PCA9685DmxServo::Stop(uint8_t nPort) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;
}

void PCA9685DmxServo::SetData(uint8_t nPort, const uint8_t* pDmxData, uint16_t nLength) {
	assert(pDmxData != 0);
	assert(nLength <= DMX_MAX_CHANNELS);

	if (__builtin_expect((m_pServo == 0), 0)) {
		Start();
	}

	uint8_t *p = (uint8_t *)pDmxData + m_nDmxStartAddress - 1;
	uint8_t *q = m_pDmxData;

	uint16_t nChannel = m_nDmxStartAddress;

	for (unsigned j = 0; j < m_nBoardInstances; j++) {
		for (unsigned i = 0; i < PCA9685_PWM_CHANNELS; i++) {
			if ((nChannel >= (m_nDmxFootprint + m_nDmxStartAddress)) || (nChannel > nLength)) {
				j = m_nBoardInstances;
				break;
			}
			if (*p != *q) {
				uint8_t value = *p;
#ifndef NDEBUG
				printf("m_pServo[%d]->SetDmx(CHANNEL(%d), %d)\n", (int) j, (int) i, (int) value);
#endif
				m_pServo[j]->Set(CHANNEL(i), value);
			}
			*q = *p;
			p++;
			q++;
			nChannel++;
		}
	}
}

void PCA9685DmxServo::SetI2cAddress(uint8_t nI2cAddress) {
	m_nI2cAddress = nI2cAddress;
}

void PCA9685DmxServo::SetBoardInstances(uint8_t nBoardInstances) {
	if ((nBoardInstances != 0) && (nBoardInstances <= BOARD_INSTANCES_MAX)) {
		m_nBoardInstances = nBoardInstances;
		m_nDmxFootprint = nBoardInstances * PCA9685_PWM_CHANNELS;
	}
}

void PCA9685DmxServo::SetLeftUs(uint16_t nLeftUs) {
	m_nLeftUs = nLeftUs;
}

void PCA9685DmxServo::SetRightUs(uint16_t nRightUs) {
	m_nRightUs = nRightUs;
}

void PCA9685DmxServo::SetDmxFootprint(uint16_t nDmxFootprint) {
	m_nDmxFootprint = nDmxFootprint;
	m_nBoardInstances = (uint16_t) ceil((float) nDmxFootprint / PCA9685_PWM_CHANNELS);
}

void PCA9685DmxServo::Initialize(void) {
	assert(m_pDmxData == 0);
	m_pDmxData = new uint8_t[m_nBoardInstances * PCA9685_PWM_CHANNELS];
	assert(m_pDmxData != 0);

	for (unsigned i = 0; i < m_nBoardInstances * PCA9685_PWM_CHANNELS; i++) {
		m_pDmxData[i] = 0;
	}

	assert(m_pServo == 0);
	m_pServo = new PCA9685Servo*[m_nBoardInstances];
	assert(m_pServo != 0);

	for (unsigned i = 0; i < m_nBoardInstances; i++) {
		m_pServo[i] = 0;
	}

	for (unsigned i = 0; i < m_nBoardInstances; i++) {
		assert(m_pServo[i] == 0);
		m_pServo[i] = new PCA9685Servo(m_nI2cAddress + i);
		assert(m_pServo[i] != 0);

		m_pServo[i]->SetLeftUs(m_nLeftUs);
		m_pServo[i]->SetRightUs(m_nRightUs);
#ifndef NDEBUG
		printf("Instance %d [%X]\n", i, m_nI2cAddress + i);
		m_pServo[i]->Dump();
		printf("\n");
#endif
	}
}


