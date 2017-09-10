/**
 * @file spisend.cpp
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <assert.h>
#include <circle/logger.h>
#include <circle/interrupt.h>

#include "ws28xxstripe.h"
#include "spisend.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifdef CLOGGER
static const char FromSPISend[] = "spisend";
#endif
/**
 *
 */
SPISend::SPISend(CInterruptSystem *pInterruptSystem) :
	m_pInterrupt (pInterruptSystem),
	m_pLEDStripe (0),
	m_LEDType (WS2801),
	m_nLEDCount (170),
	m_nBeginIndexPortId1 (170),
	m_nBeginIndexPortId2 (340),
	m_nBeginIndexPortId3 (510),
	m_nChannelsPerLed (3)
{
}

/**
 *
 */
SPISend::~SPISend(void)
{
	this->Stop();
}

/**
 *
 */
void SPISend::Start(void)
{
	assert(m_pLEDStripe == 0);
	m_pLEDStripe = new CWS28XXStripe(m_pInterrupt, m_LEDType, m_nLEDCount);
	assert(m_pLEDStripe != 0);

	m_pLEDStripe->Initialize();
	m_pLEDStripe->Blackout();
}

/**
 *
 */
void SPISend::Stop(void)
{
	while (m_pLEDStripe->IsUpdating())
	{
		// wait for completion
	}
	m_pLEDStripe->Blackout();
	delete m_pLEDStripe;
	m_pLEDStripe = 0;
}

/**
 *
 * @param nPortId
 * @param data
 * @param length
 */
void SPISend::SetData(const uint8_t nPortId, const uint8_t *data, const uint16_t length)
{
	uint16_t beginIndex = (uint16_t) 0;
	uint16_t endIndex = (uint16_t) 0;

	boolean bUpdate = false;

	switch (nPortId)
	{
	case 0:
		beginIndex = (uint16_t) 0;
		endIndex = min(m_nLEDCount, (uint16_t)(length / (uint16_t) m_nChannelsPerLed));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	case 1:
		beginIndex = (uint16_t) m_nBeginIndexPortId1;
		endIndex = min(m_nLEDCount, (uint16_t)(beginIndex + (length / (uint16_t) m_nChannelsPerLed)));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	case 2:
		beginIndex = (uint16_t) m_nBeginIndexPortId2;
		endIndex = min(m_nLEDCount, (uint16_t)(beginIndex + (length / (uint16_t) m_nChannelsPerLed)));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	case 3:
		beginIndex = (uint16_t) m_nBeginIndexPortId3;
		endIndex = min(m_nLEDCount, (uint16_t)(beginIndex + (length / (uint16_t) m_nChannelsPerLed)));
		bUpdate = (endIndex == m_nLEDCount);
		break;
	default:
		break;
	}


#ifdef CLOGGER
	CLogger::Get ()->Write(FromSPISend, LogDebug, "%u %u %u %s", nPortId, beginIndex, endIndex, bUpdate == false ? "False" : "True");
#endif

	while (m_pLEDStripe->IsUpdating ())
	{
		// wait for completion
	}

	unsigned i = 0;

	if (m_LEDType == SK6812W)
	{
		for (unsigned j = beginIndex; j < endIndex; j++)
		{
			m_pLEDStripe->SetLED(j, data[i], data[i + 1], data[i + 2], data[i + 3]);
			i = i + 4;
		}

	}
	else
	{
		for (unsigned j = beginIndex; j < endIndex; j++)
		{
			m_pLEDStripe->SetLED(j, data[i], data[i + 1], data[i + 2]);
			i = i + 3;
		}
	}

	if (bUpdate) {
		m_pLEDStripe->Update();
	}
}

/**
 *
 * @param LEDType
 */
void SPISend::SetLEDType(TWS28XXType LEDType)
{
	m_LEDType = LEDType;

	if (LEDType == SK6812W) {
		m_nBeginIndexPortId1 = 128;
		m_nBeginIndexPortId2 = 256;
		m_nBeginIndexPortId3 = 384;

		m_nChannelsPerLed = 4;
	}
}

/**
 *
 * @return
 */
const TWS28XXType SPISend::GetLEDType(void) {
	return m_LEDType;
}

/**
 *
 * @param nLEDCount
 */
void SPISend::SetLEDCount(unsigned nLEDCount)
{
	m_nLEDCount = nLEDCount;
}

/**
 *
 */
unsigned SPISend::GetLEDCount(void)
{
	return m_nLEDCount;
}
