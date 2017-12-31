/**
 * @file servodmx.cpp
 *
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
#include <stdio.h>
#include <assert.h>

#include "servodmx.h"

#include "servo.h"
#include "pca9685.h"

#define DMX_MAX_CHANNELS	512

ServoDMX::ServoDMX(void):
	m_nDmxStartAddress(1),
	m_nI2cAddress(PCA9685_I2C_ADDRESS_DEFAULT),
	m_nBoardInstances(1),
	m_nLeftUs(SERVO_LEFT_DEFAULT_US),
	m_nRightUs(SERVO_RIGHT_DEFAULT_US),
	m_bIsStarted(false),
	m_pServo(0),
	m_pDmxData(0)
{

}

ServoDMX::~ServoDMX(void) {
	delete m_pServo;
	m_pServo = 0;
}

void ServoDMX::Start(void) {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	if (m_pServo == 0) {
		Initialize();
	}
}

void ServoDMX::Stop(void) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;
}

void ServoDMX::SetData(uint8_t nPort, const uint8_t *pDmxData, uint16_t nLength) {
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
			if (nChannel > nLength) {
				j = m_nBoardInstances;
				break;
			}
			if (*p != *q) {
				uint8_t value = *p;
#ifndef NDEBUG
				printf("m_pServo[%d]->SetDmx(CHANNEL(%d), %d)\n", (int) j, (int) i, (int) value);
#endif
				m_pServo[j]->SetDmx(CHANNEL(i), value);
			}
			*q = *p;
			p++;
			q++;
			nChannel++;
		}
	}
}

uint16_t ServoDMX::GetDmxStartAddress(void) const {
	return m_nDmxStartAddress;
}

void ServoDMX::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= 512));

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= 512)) {
		m_nDmxStartAddress = nDmxStartAddress;
	}
}

void ServoDMX::SetI2cAddress(uint8_t nI2cAddress) {
	m_nI2cAddress = nI2cAddress;
}

void ServoDMX::SetBoardInstances(uint8_t nBoardInstances) {
	if ((nBoardInstances != 0) && (nBoardInstances <= 64)) { //TODO #define
		m_nBoardInstances = nBoardInstances;
	}
}

void ServoDMX::SetLeftUs(uint16_t nLeftUs) {
	m_nLeftUs = nLeftUs;
}

void ServoDMX::SetRightUs(uint16_t nRightUs) {
	m_nRightUs = nRightUs;
}

void ServoDMX::Initialize(void) {
	assert(m_pDmxData == 0);
	m_pDmxData = new uint8_t[m_nBoardInstances * PCA9685_PWM_CHANNELS];
	assert(m_pDmxData != 0);

	for (unsigned i = 0; i < m_nBoardInstances * PCA9685_PWM_CHANNELS; i++) {
		m_pDmxData[i] = 0;
	}

	assert(m_pServo == 0);
	m_pServo = new Servo*[m_nBoardInstances];
	assert(m_pServo != 0);

	for (unsigned i = 0; i < m_nBoardInstances; i++) {
		m_pServo[i] = 0;
	}

	for (unsigned i = 0; i < m_nBoardInstances; i++) {
		assert(m_pServo[i] == 0);
		m_pServo[i] = new Servo(m_nI2cAddress + i);
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
