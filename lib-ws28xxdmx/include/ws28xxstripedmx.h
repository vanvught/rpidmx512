/**
 * @file spisend.h
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef SPISEND_H_
#define SPISEND_H_

#include <stdint.h>

#if defined (__circle__)
#include "circle/interrupt.h"
#endif

#include "lightset.h"

#include "ws28xxstripe.h"

class SPISend: public LightSet {
public:
#if defined (__circle__)
	SPISend(CInterruptSystem *);
#else
	SPISend(void);
#endif
	~SPISend(void);

	void Start(uint8_t nPort = 0);
	void Stop(uint8_t nPort = 0);

	void SetData(uint8_t nPort, const uint8_t *, uint16_t);

	void SetLEDType(const TWS28XXType);
	TWS28XXType GetLEDType(void) const;

	void SetLEDCount(uint16_t);
	uint16_t GetLEDCount(void) const;

	void Print(void);

public: // RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress);

	inline uint16_t GetDmxStartAddress(void) {
		return m_nDmxStartAddress;
	}

	inline uint16_t GetDmxFootprint(void) {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo);

#if defined (__circle__)
private:
	CInterruptSystem	*m_pInterrupt;
#endif

private:
	void UpdateMembers(void);

private:
	uint16_t m_nDmxStartAddress;
	uint16_t m_nDmxFootprint;

	bool m_bIsStarted;

	WS28XXStripe	*m_pLEDStripe;
	TWS28XXType		m_LEDType;
	uint16_t		m_nLEDCount;

	uint16_t		m_nBeginIndexPortId1;
	uint16_t		m_nBeginIndexPortId2;
	uint16_t		m_nBeginIndexPortId3;

	uint16_t		m_nChannelsPerLed;
};

#endif /* SPISEND_H_ */
