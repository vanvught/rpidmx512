/**
 * @file pwmleddmx.cpp
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

#include "pwmleddmx.h"

#include "pwmled.h"
#include "pca9685.h"

#define DMX_MAX_CHANNELS	512
#define BOARD_INSTANCES_MAX	32

PWMLedDMX::PWMLedDMX(void) :
	m_nDmxStartAddress(1),
	m_nI2cAddress(PCA9685_I2C_ADDRESS_DEFAULT),
	m_nBoardInstances(1),
	m_nPwmFrequency(PWMLED_DEFAULT_FREQUENCY),
	m_bOutputInvert(false), // Output logic state not inverted. Value to use when external driver used.
	m_bOutputDriver(true),	// The 16 LEDn outputs are configured with a totem pole structure.
	m_bIsStarted(false),
	m_pPWMLed(0),
	m_pDmxData(0)
{

}

PWMLedDMX::~PWMLedDMX(void) {
	delete[] m_pDmxData;
	m_pDmxData = 0;

	for (unsigned i = 0; i < m_nBoardInstances; i++) {
		delete m_pPWMLed[i];
		m_pPWMLed[i] = 0;
	}

	delete[] m_pPWMLed;
	m_pPWMLed = 0;
}

void PWMLedDMX::Start(void) {
	if (m_bIsStarted) {
		return;
	}

	m_bIsStarted = true;

	if (m_pPWMLed == 0) {
		Initialize();
	}
}

void PWMLedDMX::Stop(void) {
	if (!m_bIsStarted) {
		return;
	}

	m_bIsStarted = false;
}

void PWMLedDMX::SetData(uint8_t nPort, const uint8_t *pDmxData, uint16_t nLength) {
	assert(pDmxData != 0);
	assert(nLength <= DMX_MAX_CHANNELS);

	if (__builtin_expect((m_pPWMLed == 0), 0)) {
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
				printf("m_pPWMLed[%d]->SetDmx(CHANNEL(%d), %d)\n", (int) j, (int) i, (int) value);
#endif
				m_pPWMLed[j]->SetDmx(CHANNEL(i), value);
			}
			*q = *p;
			p++;
			q++;
			nChannel++;
		}
	}
}

uint16_t PWMLedDMX::GetDmxStartAddress(void) const {
	return m_nDmxStartAddress;
}

void PWMLedDMX::SetDmxStartAddress(uint16_t nDmxStartAddress) {
	assert((nDmxStartAddress != 0) && (nDmxStartAddress <= DMX_MAX_CHANNELS));

	if ((nDmxStartAddress != 0) && (nDmxStartAddress <= DMX_MAX_CHANNELS)) {
		m_nDmxStartAddress = nDmxStartAddress;
	}
}

uint8_t PWMLedDMX::GetI2cAddress(void) const {
	return m_nI2cAddress;
}

void PWMLedDMX::SetI2cAddress(uint8_t nI2cAddress) {
	m_nI2cAddress = nI2cAddress;
}

void PWMLedDMX::SetBoardInstances(uint8_t nBoardInstances) {
	if ((nBoardInstances != 0) && (nBoardInstances <= BOARD_INSTANCES_MAX)) {
		m_nBoardInstances = nBoardInstances;
	}
}

void PWMLedDMX::SetPwmfrequency(uint16_t nPwmfrequency) {
	m_nPwmFrequency = nPwmfrequency;
}

bool PWMLedDMX::GetInvert(void) const {
	return m_bOutputInvert;
}

void PWMLedDMX::SetInvert(bool bOutputInvert) {
	m_bOutputInvert = bOutputInvert;
}

bool PWMLedDMX::GetOutDriver(void) const {
	return m_bOutputDriver;
}

void PWMLedDMX::SetOutDriver(bool bOutputDriver) {
	m_bOutputDriver= bOutputDriver;
}

void PWMLedDMX::Initialize(void) {
	assert(m_pDmxData == 0);
	m_pDmxData = new uint8_t[m_nBoardInstances * PCA9685_PWM_CHANNELS];
	assert(m_pDmxData != 0);

	for (unsigned i = 0; i < m_nBoardInstances * PCA9685_PWM_CHANNELS; i++) {
		m_pDmxData[i] = 0;
	}

	assert(m_pPWMLed == 0);
	m_pPWMLed = new PWMLed*[m_nBoardInstances];
	assert(m_pPWMLed != 0);

	for (unsigned i = 0; i < m_nBoardInstances; i++) {
		m_pPWMLed[i] = 0;
	}

	for (unsigned i = 0; i < m_nBoardInstances; i++) {
		assert(m_pPWMLed[i] == 0);
		m_pPWMLed[i] = new PWMLed(m_nI2cAddress + i);
		assert(m_pPWMLed[i] != 0);

		m_pPWMLed[i]->SetInvert(m_bOutputInvert);
		m_pPWMLed[i]->SetOutDriver(m_bOutputDriver);
		m_pPWMLed[i]->SetFrequency(m_nPwmFrequency);
		m_pPWMLed[i]->SetFullOff(CHANNEL(16), true);
#ifndef NDEBUG
		printf("Instance %d [%X]\n", i, m_nI2cAddress + i);
		m_pPWMLed[i]->Dump();
		printf("\n");
#endif
	}
}
