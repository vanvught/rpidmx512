/**
 * @file l6470dmxmodes.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#ifndef NDEBUG
 #include <cstdio>
#endif
#include <cassert>

#include "l6470.h"

#include "l6470dmxmodes.h"
#include "l6470dmxmode0.h"
#include "l6470dmxmode1.h"
#include "l6470dmxmode2.h"
#include "l6470dmxmode3.h"
#include "l6470dmxmode4.h"
#include "l6470dmxmode5.h"
#include "l6470dmxmode6.h"

#include "motorparams.h"
#include "modeparams.h"

#include "lightset.h"

#include "debug.h"

L6470DmxModes::L6470DmxModes(TL6470DmxModes tMode, uint16_t nDmxStartAddress, L6470 *pL6470, MotorParams *pMotorParams, ModeParams *pModeParams) {
	DEBUG_ENTRY;

	assert(nDmxStartAddress <= lightset::dmx::UNIVERSE_SIZE);
	assert(pL6470 != nullptr);
	assert(pMotorParams != nullptr);
	assert(pModeParams != nullptr);

	m_nDmxStartAddress = nDmxStartAddress;

	switch (tMode) {
		case L6470DMXMODE0:
			m_pDmxMode = new L6470DmxMode0(pL6470);
			m_DmxFootPrint = L6470DmxMode0::GetDmxFootPrint();
			break;
		case L6470DMXMODE1:
			m_pDmxMode = new L6470DmxMode1(pL6470);
			m_DmxFootPrint = L6470DmxMode1::GetDmxFootPrint();
			break;
		case L6470DMXMODE2:
			m_pDmxMode = new L6470DmxMode2(pL6470);
			m_DmxFootPrint = L6470DmxMode2::GetDmxFootPrint();
			break;
		case L6470DMXMODE3:
			m_pDmxMode = new L6470DmxMode3(pL6470, pMotorParams);
			m_DmxFootPrint = L6470DmxMode3::GetDmxFootPrint();
			break;
		case L6470DMXMODE4:
			m_pDmxMode = new L6470DmxMode4(pL6470, pMotorParams, pModeParams);
			m_DmxFootPrint = L6470DmxMode4::GetDmxFootPrint();
			break;
		case L6470DMXMODE5:
			m_pDmxMode = new L6470DmxMode5(pL6470, pMotorParams, pModeParams);
			m_DmxFootPrint = L6470DmxMode5::GetDmxFootPrint();
			break;
		case L6470DMXMODE6:
			m_pDmxMode = new L6470DmxMode6(pL6470, pMotorParams, pModeParams);
			m_DmxFootPrint = L6470DmxMode6::GetDmxFootPrint();
			break;
		default:
			break;
	}

	assert(m_pDmxMode != nullptr);

	if (m_pDmxMode != nullptr) {
		m_nMotorNumber = static_cast<uint8_t>(pL6470->GetMotorNumber());
		m_nMode = tMode;

		m_pDmxData = new uint8_t[m_DmxFootPrint];
		assert(m_pDmxData != nullptr);

		for (int i = 0; i < m_DmxFootPrint; i++) {
			m_pDmxData[i] = 0;
		}
	}

	DEBUG_EXIT;
}

L6470DmxModes::~L6470DmxModes() {
	DEBUG_ENTRY;

	delete [] m_pDmxData;
	m_pDmxData = nullptr;

	delete m_pDmxMode;
	m_pDmxMode = nullptr;

	DEBUG_EXIT;
}

void L6470DmxModes::InitSwitch() {
	DEBUG_ENTRY;

	m_pDmxMode->InitSwitch();

	DEBUG_EXIT
}

void L6470DmxModes::InitPos() {
	DEBUG_ENTRY;

	m_pDmxMode->InitPos();

	DEBUG_EXIT
}

uint16_t L6470DmxModes::GetDmxFootPrintMode(uint32_t tMode) {
	switch (tMode) {
		case L6470DMXMODE0:
			return L6470DmxMode0::GetDmxFootPrint();
			break;
		case L6470DMXMODE1:
			return L6470DmxMode1::GetDmxFootPrint();
			break;
		case L6470DMXMODE2:
			return L6470DmxMode2::GetDmxFootPrint();
			break;
		case L6470DMXMODE3:
			return L6470DmxMode3::GetDmxFootPrint();
			break;
		case L6470DMXMODE4:
			return L6470DmxMode4::GetDmxFootPrint();
			break;
		case L6470DMXMODE5:
			return L6470DmxMode5::GetDmxFootPrint();
			break;
		case L6470DMXMODE6:
			return L6470DmxMode6::GetDmxFootPrint();
			break;
		default:
			return 0;
			break;
	}
}

void L6470DmxModes::Start() {
	DEBUG_ENTRY;

	if (m_bIsStarted) {
		return;
	}

	m_pDmxMode->Start();

	m_bIsStarted = true;

	DEBUG_EXIT;
}

void L6470DmxModes::Stop() {
	DEBUG_ENTRY;

	if (!m_bIsStarted) {
		return;
	}

	m_pDmxMode->Stop();

	m_bIsStarted = false;

	DEBUG_EXIT;
}

void L6470DmxModes::HandleBusy() {
	DEBUG_ENTRY;

	m_pDmxMode->HandleBusy();

	DEBUG_EXIT;
}

bool L6470DmxModes::BusyCheck() {
	DEBUG_ENTRY;

	DEBUG_EXIT;
	return m_pDmxMode->BusyCheck();
}

bool L6470DmxModes::IsDmxDataChanged(const uint8_t *p) {
	DEBUG_ENTRY;

	auto isChanged = false;
	const auto lastDmxChannel = static_cast<uint16_t>(m_nDmxStartAddress + m_DmxFootPrint - 1);
	auto *q = m_pDmxData;

#ifndef NDEBUG
	printf("\t\tDmxStartAddress = %d, LastDmxChannel = %d\n", m_nDmxStartAddress, lastDmxChannel);
#endif

	for (uint32_t i = m_nDmxStartAddress; (i <= lastDmxChannel) && (i <= 512U) ; i++) {
		if (*p != *q) {
			isChanged = true;
		}
		*q = *p;
		p++;
		q++;
	}

	DEBUG_EXIT;
	return isChanged;
}

bool L6470DmxModes::IsDmxDataChanged(const uint8_t *pDmxData, uint32_t nLength) {
	DEBUG_ENTRY;

	assert(m_pDmxMode != nullptr);
	assert(pDmxData != nullptr);

	if (m_pDmxMode == nullptr) {
		DEBUG_EXIT;
		return false;
	}

	if (nLength < (m_nDmxStartAddress + m_DmxFootPrint)) {
		DEBUG_EXIT;
		return false;
	}

	uint8_t *p = const_cast<uint8_t *>(pDmxData) + m_nDmxStartAddress - 1;

	return IsDmxDataChanged(p);
}

void L6470DmxModes::DmxData(const uint8_t *pDmxData, uint32_t nLength) {
	DEBUG_ENTRY;

	assert(m_pDmxMode != nullptr);
	assert(pDmxData != nullptr);

	if (nLength < (m_nDmxStartAddress + m_DmxFootPrint)) {
		return;
	}

	const auto *p = const_cast<uint8_t *>(pDmxData) + m_nDmxStartAddress - 1;

#ifndef NDEBUG
	printf("\tMotor : %d\n", m_nMotorNumber);

	for (uint32_t i = 0; i < m_DmxFootPrint; i++) {
		printf("\t\tDMX slot(%d) : %d\n", m_nDmxStartAddress + i, p[i]);
	}
#endif


	m_pDmxMode->Data(p);

	m_bIsStarted = true;

	DEBUG_EXIT;
}

