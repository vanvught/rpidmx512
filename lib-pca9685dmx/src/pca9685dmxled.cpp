/**
 * @file pca9685dmxled.cpp
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "pca9685dmxled.h"

#include "debug.h"

PCA9685DmxLed::PCA9685DmxLed(const pca9685dmx::Configuration& configuration) {
	DEBUG_ENTRY

	m_nBoardInstances = static_cast<uint8_t>((configuration.nChannelCount + (pca9685::PWM_CHANNELS - 1))  / pca9685::PWM_CHANNELS);
	m_nDmxFootprint = configuration.nChannelCount;
	m_nDmxStartAddress = configuration.nDmxStartAddress;

	m_pDmxData = new uint8_t[m_nDmxFootprint];
	assert(m_pDmxData != nullptr);

	for (uint32_t i = 0; i < m_nDmxFootprint; i++) {
		m_pDmxData[i] = 0;
	}

	m_pPWMLed = new PCA9685PWMLed*[m_nBoardInstances];
	assert(m_pPWMLed != nullptr);

	for (unsigned i = 0; i < m_nBoardInstances; i++) {
		m_pPWMLed[i] = new PCA9685PWMLed(static_cast<uint8_t>(configuration.nAddress + i));
		assert(m_pPWMLed[i] != nullptr);

		m_pPWMLed[i]->SetInvert(configuration.led.invert);
		m_pPWMLed[i]->SetOutDriver(configuration.led.output);
		m_pPWMLed[i]->SetFrequency(configuration.led.nLedPwmFrequency);
		m_pPWMLed[i]->SetFullOff(CHANNEL(16), true);
#ifndef NDEBUG
		printf("Instance %d [%X]\n", i, configuration.nAddress + i);
		m_pPWMLed[i]->Dump();
		printf("\n");
#endif
	}
}

PCA9685DmxLed::~PCA9685DmxLed() {
	DEBUG_ENTRY

	delete[] m_pDmxData;
	m_pDmxData = nullptr;

	for (uint32_t i = 0; i < m_nBoardInstances; i++) {
		delete m_pPWMLed[i];
		m_pPWMLed[i] = nullptr;
	}

	delete[] m_pPWMLed;
	m_pPWMLed = nullptr;

	DEBUG_EXIT
}

void PCA9685DmxLed::SetData(__attribute__((unused)) uint32_t nPortIndex, const uint8_t *pDmxData, uint32_t nLength, __attribute__((unused)) const bool doUpdate) {
	assert(pDmxData != nullptr);
	assert(nLength <= lightset::dmx::UNIVERSE_SIZE);

	auto *p = const_cast<uint8_t*>(pDmxData) + m_nDmxStartAddress - 1;
	auto *q = m_pDmxData;
	auto nChannel = m_nDmxStartAddress;

	for (uint32_t j = 0; j < m_nBoardInstances; j++) {
		for (uint32_t i = 0; i < pca9685::PWM_CHANNELS; i++) {
			if ((nChannel >= (m_nDmxFootprint + m_nDmxStartAddress)) || (nChannel > nLength)) {
				j = m_nBoardInstances;
				break;
			}
			if (*p != *q) {
				uint8_t value = *p;
#ifndef NDEBUG
				printf("m_pPWMLed[%u]->SetDmx(CHANNEL(%u), %u)\n", static_cast<uint32_t>(j), static_cast<uint32_t>(i), static_cast<uint32_t>(value));
#endif
				m_pPWMLed[j]->Set(CHANNEL(i), value);
			}
			*q = *p;
			p++;
			q++;
			nChannel++;
		}
	}
}

bool PCA9685DmxLed::GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo& tSlotInfo) {
	if (nSlotOffset >  m_nDmxFootprint) {
		return false;
	}

	tSlotInfo.nType = 0;
	tSlotInfo.nCategory = 0;

	return true;
}

void PCA9685DmxLed::Print() {
	DEBUG_ENTRY

//	prinff("DMX Footprint : %u\n", m_nDmxFootprint);
//	m_nDmxStartAddress
//	m_nBoardInstances

	DEBUG_EXIT
}
