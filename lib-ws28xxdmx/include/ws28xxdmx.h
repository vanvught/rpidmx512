/**
 * @file ws28xxdmx.h
 *
 */
/* Copyright (C) 2016-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef WS28XXDMX_H_
#define WS28XXDMX_H_

#include <stdint.h>

#if defined (__circle__)
 #include "circle/interrupt.h"
#endif

#include "lightset.h"

#include "ws28xx.h"

class WS28xxDmx: public LightSet {
public:
#if defined (__circle__)
	WS28xxDmx(CInterruptSystem *);
#else
	WS28xxDmx(void);
#endif
	virtual ~WS28xxDmx(void);

	void Start(uint8_t nPort = 0);
	void Stop(uint8_t nPort = 0);

	virtual void SetData(uint8_t nPort, const uint8_t *, uint16_t);

	void Blackout(bool bBlackout);

	virtual void SetLEDType(TWS28XXType);
	 TWS28XXType GetLEDType(void) {
		return m_tLedType;
	}

	virtual void SetLEDCount(uint16_t);
	 uint16_t GetLEDCount(void) {
		return m_nLedCount;
	}

	void SetClockSpeedHz(uint32_t nClockSpeedHz) {
		m_nClockSpeedHz = nClockSpeedHz;
	}

	uint32_t GetClockSpeedHz(void) const {
		return m_nClockSpeedHz;
	}

	void SetGlobalBrightness(uint8_t nGlobalBrightness) {
		m_nGlobalBrightness = nGlobalBrightness;
	}

	uint8_t GetGlobalBrightness(void) const {
		return m_nGlobalBrightness;
	}

	virtual void Print(void);

public: // RDM
	virtual bool SetDmxStartAddress(uint16_t nDmxStartAddress);

	uint16_t GetDmxStartAddress(void) {
		return m_nDmxStartAddress;
	}

	uint16_t GetDmxFootprint(void) {
		return m_nDmxFootprint;
	}

	virtual bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo);

#if defined (__circle__)
private:
	CInterruptSystem	*m_pInterrupt;
#endif

private:
	void UpdateMembers(void);

protected:
	TWS28XXType m_tLedType;
	uint16_t m_nLedCount;
	uint16_t m_nDmxStartAddress;
	uint16_t m_nDmxFootprint;

	WS28xx* m_pLEDStripe;
	bool m_bIsStarted;
	bool m_bBlackout;

private:
	uint32_t m_nClockSpeedHz;
	uint8_t m_nGlobalBrightness;
	uint32_t m_nBeginIndexPortId1;
	uint32_t m_nBeginIndexPortId2;
	uint32_t m_nBeginIndexPortId3;
	uint32_t m_nChannelsPerLed;

	uint32_t m_nPortIdLast;
};

#endif /* WS28XXDMX_H_ */
