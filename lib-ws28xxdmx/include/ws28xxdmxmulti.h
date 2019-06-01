/**
 * @file ws28xxdmxmulti.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef WS28XXDMXMULTI_H_
#define WS28XXDMXMULTI_H_

#include <stdint.h>

#include "lightset.h"

#include "ws28xxmulti.h"

enum TWS28xxDmxMultiSrc {
	WS28XXDMXMULTI_SRC_ARTNET,
	WS28XXDMXMULTI_SRC_E131
};

class WS28xxDmxMulti: public LightSet {
public:
	WS28xxDmxMulti(TWS28xxDmxMultiSrc tSrc);
	virtual ~WS28xxDmxMulti(void);

	void Start(uint8_t nPort);
	void Stop(uint8_t nPort);

	void SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLength);

	void Blackout(bool bBlackout);

	virtual void SetLEDType(TWS28xxMultiType tWS28xxMultiType);
	TWS28xxMultiType GetLEDType(void) {
		return m_tLedType;
	}

	void SetLEDCount(uint16_t nLedCount);
	uint32_t GetLEDCount(void) {
		return m_nLedCount;
	}

	void SetActivePorts(uint8_t nActiveOutputs);
	uint32_t GetActivePorts(void) {
		return m_nActiveOutputs;
	}

	uint32_t GetUniverses(void) {
		return m_nUniverses;
	}

	void SetUseSI5351A(bool bUse) {
		m_bUseSI5351A = bUse;
	}
	bool GetUseSI5351A(void) {
		return m_bUseSI5351A;
	}

	void Print(void);

private:
	void UpdateMembers(void);

private:
	TWS28xxDmxMultiSrc m_tSrc;
	TWS28xxMultiType m_tLedType;
	uint32_t m_nLedCount;
	uint32_t m_nActiveOutputs;

	WS28xxMulti* m_pLEDStripe;

	bool m_bIsStarted;
	bool m_bBlackout;

	uint32_t m_nUniverses;

	uint32_t m_nBeginIndexPortId1;
	uint32_t m_nBeginIndexPortId2;
	uint32_t m_nBeginIndexPortId3;
	uint32_t m_nChannelsPerLed;

	uint32_t m_nPortIdLast;
	bool m_bUseSI5351A;
};

#endif /* WS28XXDMXMULTI_H_ */
