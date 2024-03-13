/**
 * @file pca9685dmxled.cpp
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include "lightset.h"

#include "debug.h"

PCA9685DmxLed::PCA9685DmxLed(const pca9685dmx::Configuration& configuration) {
	DEBUG_ENTRY

	m_bUse8Bit = configuration.bUse8Bit;
	m_nChannelCount = configuration.nChannelCount;

	if (m_bUse8Bit) {
		m_nDmxFootprint = m_nChannelCount;
	} else {
		if (m_nChannelCount > lightset::dmx::UNIVERSE_SIZE / 2) {
			m_nChannelCount = lightset::dmx::UNIVERSE_SIZE / 2;
		}
		m_nDmxFootprint = 2 * m_nChannelCount;
	}

	DEBUG_PRINTF("m_bUse8Bit=%c, m_nChannelCount=%u, m_nDmxFootprint=%u", m_bUse8Bit ? 'Y' : 'N', m_nChannelCount, m_nDmxFootprint);

	m_nBoardInstances = static_cast<uint8_t>((m_nChannelCount + (pca9685::PWM_CHANNELS - 1))  / pca9685::PWM_CHANNELS);

	DEBUG_PRINTF("m_nBoardInstances=%u", m_nBoardInstances);

	m_nDmxStartAddress = configuration.nDmxStartAddress;

	memset(m_DmxData, 0, sizeof(m_DmxData));

	m_pPWMLed = new PCA9685PWMLed*[m_nBoardInstances];
	assert(m_pPWMLed != nullptr);

	for (uint32_t i = 0; i < m_nBoardInstances; i++) {
		m_pPWMLed[i] = new PCA9685PWMLed(static_cast<uint8_t>(configuration.nAddress + i));
		assert(m_pPWMLed[i] != nullptr);

		m_pPWMLed[i]->SetInvert(configuration.led.invert);
		m_pPWMLed[i]->SetOutDriver(configuration.led.output);
		m_pPWMLed[i]->SetFrequency(configuration.led.nLedPwmFrequency);
		m_pPWMLed[i]->SetFullOff(CHANNEL(16), true);
#ifndef NDEBUG
		printf("Instance %d [%X]\n", i, configuration.nAddress + i);
		m_pPWMLed[i]->Dump();
		puts("");
#endif
	}

	DEBUG_EXIT
}

PCA9685DmxLed::~PCA9685DmxLed() {
	DEBUG_ENTRY

	for (uint32_t i = 0; i < m_nBoardInstances; i++) {
		delete m_pPWMLed[i];
		m_pPWMLed[i] = nullptr;
	}

	delete[] m_pPWMLed;
	m_pPWMLed = nullptr;

	DEBUG_EXIT
}

void PCA9685DmxLed::Stop([[maybe_unused]] const uint32_t nPortIndex) {
	DEBUG_ENTRY

	for (uint32_t j = 0; j < m_nBoardInstances; j++) {
		m_pPWMLed[j]->SetFullOff(CHANNEL(16), true);
	}

	DEBUG_EXIT
}

void PCA9685DmxLed::SetData([[maybe_unused]] uint32_t nPortIndex, const uint8_t *pDmxData, uint32_t nLength, [[maybe_unused]] const bool doUpdate) {
	assert(pDmxData != nullptr);
	assert(nLength <= lightset::dmx::UNIVERSE_SIZE);

	auto nDmxAddress = m_nDmxStartAddress;

	if (m_bUse8Bit) {
		auto *pCurrentData = const_cast<uint8_t *>(pDmxData) + m_nDmxStartAddress - 1;
		auto *pPreviousData = m_DmxData;

		for (uint32_t j = 0; j < m_nBoardInstances; j++) {
			for (uint32_t i = 0; i < pca9685::PWM_CHANNELS; i++) {
				if ((nDmxAddress >= (m_nDmxFootprint + m_nDmxStartAddress)) || (nDmxAddress > nLength)) {
					j = m_nBoardInstances;
					break;
				}
				if (*pCurrentData != *pPreviousData) {
					*pPreviousData = *pCurrentData;
					const auto value = *pCurrentData;
#ifndef NDEBUG
					printf("m_pPWMLed[%u]->SetDmx(CHANNEL(%u), %u)\n", j, i, static_cast<uint32_t>(value));
#endif
					m_pPWMLed[j]->Set(i, value);
				}
				pCurrentData++;
				pPreviousData++;
				nDmxAddress++;
			}
		}
	} else {
		auto *pCurrentData = reinterpret_cast<const uint16_t *>(pDmxData + m_nDmxStartAddress - 1);
		auto *pPreviousData = reinterpret_cast<uint16_t *>(m_DmxData);

		for (uint32_t j = 0; j < m_nBoardInstances; j++) {
			for (uint32_t i = 0; i < pca9685::PWM_CHANNELS; i++) {
				if ((nDmxAddress >= (m_nDmxFootprint + m_nDmxStartAddress)) || (nDmxAddress > nLength)) {
					j = m_nBoardInstances;
					break;
				}

				if (*pCurrentData != *pPreviousData) {
					*pPreviousData = *pCurrentData;
					const auto *pData = reinterpret_cast<const uint8_t *>(pCurrentData);
					const auto value = static_cast<uint16_t>((static_cast<uint32_t>(pData[0]) << 4) | static_cast<uint32_t>(pData[1]));
#ifndef NDEBUG
					printf("m_pPWMLed[%u]->SetDmx(CHANNEL(%u), %u)\n", j, i, static_cast<uint32_t>(value));
#endif
					m_pPWMLed[j]->Set(i, value);
				}
				pCurrentData++;
				pPreviousData++;
				nDmxAddress+=2;
			}
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
	printf("PCA9685 LED %d-bit\n", m_bUse8Bit ? 8 : 16);
	printf(" Board instances: %u\n", m_nBoardInstances);
	printf(" Channel count: %u\n", m_nChannelCount);
	printf(" DMX start address: %u [footprint: %u]\n", m_nDmxStartAddress, m_nDmxFootprint);
}
