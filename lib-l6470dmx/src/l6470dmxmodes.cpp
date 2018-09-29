/**
 * @file l6470dmxmodes.cpp
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

#include <stdint.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

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

#include "debug.h"

L6470DmxModes::L6470DmxModes(TL6470DmxModes tMode, uint16_t nDmxStartAddress, L6470 *pL6470, MotorParams *pMotorParams, ModeParams *pModeParams): m_bIsStarted(false), m_nMotorNumber(0), m_nMode(L6470DMXMODE_UNDEFINED), m_pDmxMode(0), m_DmxFootPrint(0) {
	DEBUG1_ENTRY;

	assert(nDmxStartAddress <= 512);
	assert(pL6470 != 0);
	assert(pMotorParams != 0);
	assert(pModeParams != 0);

	m_nDmxStartAddress = nDmxStartAddress;

	switch (tMode) {
		case L6470DMXMODE0:
			m_pDmxMode = new L6470DmxMode0(pL6470, pMotorParams);
			m_DmxFootPrint = L6470DmxMode0::GetDmxFootPrint();
			break;
		case L6470DMXMODE1:
			m_pDmxMode = new L6470DmxMode1(pL6470, pMotorParams);
			m_DmxFootPrint = L6470DmxMode1::GetDmxFootPrint();
			break;
		case L6470DMXMODE2:
			m_pDmxMode = new L6470DmxMode2(pL6470, pMotorParams);
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

	assert(m_pDmxMode != 0);

	if (m_pDmxMode != 0) {
		m_nMotorNumber = pL6470->GetMotorNumber();
		m_nMode = tMode;

		m_pDmxData = new uint8_t[m_DmxFootPrint];
		assert(m_pDmxData != 0);

		for (int i = 0; i < m_DmxFootPrint; i++) {
			m_pDmxData[i] = 0;
		}
	}

	DEBUG1_EXIT;
}

L6470DmxModes::~L6470DmxModes(void) {
	DEBUG1_ENTRY;

	delete [] m_pDmxData;
	m_pDmxData = 0;

	delete m_pDmxMode;
	m_pDmxMode = 0;

	DEBUG1_EXIT;
}

void L6470DmxModes::InitSwitch(void) {
	DEBUG1_ENTRY;

	m_pDmxMode->InitSwitch();

	DEBUG1_EXIT
}

void L6470DmxModes::InitPos(void) {
	DEBUG1_ENTRY;

	m_pDmxMode->InitPos();

	DEBUG1_EXIT
}

uint8_t L6470DmxModes::GetDmxFootPrintMode(uint8_t tMode) {
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

void L6470DmxModes::Start(void) {
	DEBUG1_ENTRY;

	if (m_bIsStarted) {
		return;
	}

	m_pDmxMode->Start();

	m_bIsStarted = true;

	DEBUG1_EXIT;
}

void L6470DmxModes::Stop(void) {
	DEBUG1_ENTRY;

	if (!m_bIsStarted) {
		return;
	}

	m_pDmxMode->Stop();

	m_bIsStarted = false;

	DEBUG1_EXIT;
}

void L6470DmxModes::HandleBusy(void) {
	DEBUG1_ENTRY;

	m_pDmxMode->HandleBusy();

	DEBUG1_EXIT;
}

bool L6470DmxModes::BusyCheck(void) {
	DEBUG1_ENTRY;

	DEBUG1_EXIT;
	return m_pDmxMode->BusyCheck();
}

bool L6470DmxModes::IsDmxDataChanged(const uint8_t *p) {
	DEBUG1_ENTRY;

	bool isChanged = false;
	uint16_t lastDmxChannel = m_nDmxStartAddress + m_DmxFootPrint - 1;
	uint8_t *q = m_pDmxData;

#ifndef NDEBUG
	printf("\t\tDmxStartAddress = %d, LastDmxChannel = %d\n", m_nDmxStartAddress, lastDmxChannel);
#endif

	for (uint16_t i = m_nDmxStartAddress; (i <= lastDmxChannel) && (i <= 512) ; i++) {
		if (*p != *q) {
			isChanged = true;
		}
		*q = *p;
		p++;
		q++;
	}

	DEBUG1_EXIT;
	return isChanged;
}

bool L6470DmxModes::IsDmxDataChanged(const uint8_t *pDmxData, uint16_t nLength) {
	DEBUG1_ENTRY;

	assert(m_pDmxMode != 0);
	assert(pDmxData != 0);

	if (m_pDmxMode == 0) {
		DEBUG1_EXIT;
		return false;
	}

	if (nLength < (m_nDmxStartAddress + m_DmxFootPrint)) {
		DEBUG1_EXIT;
		return false;
	}

	uint8_t *p = (uint8_t *)pDmxData + m_nDmxStartAddress - 1;

	return IsDmxDataChanged(p);
}

void L6470DmxModes::DmxData(const uint8_t *pDmxData, uint16_t nLength) {
	DEBUG1_ENTRY;

	assert(m_pDmxMode != 0);
	assert(pDmxData != 0);

	if (nLength < (m_nDmxStartAddress + m_DmxFootPrint)) {
		return;
	}

	uint8_t *p = (uint8_t *)pDmxData + m_nDmxStartAddress - 1;

#ifndef NDEBUG
	printf("\tMotor : %d\n", m_nMotorNumber);

	for (uint8_t i = 0; i < m_DmxFootPrint; i++) {
		printf("\t\tDMX slot(%d) : %d\n", m_nDmxStartAddress + i, p[i]);
	}
#endif


	m_pDmxMode->Data(p);

	m_bIsStarted = true;

	DEBUG1_EXIT;
}

