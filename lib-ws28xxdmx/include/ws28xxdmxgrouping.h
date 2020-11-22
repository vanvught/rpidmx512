/**
 * @file ws28xxdmxgrouping.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef WS28XXDMXGROUPING_H_
#define WS28XXDMXGROUPING_H_

#include <stdint.h>

#include "ws28xxdmx.h"
#include "ws28xx.h"

class WS28xxDmxGrouping final: public WS28xxDmx {
public:
	WS28xxDmxGrouping();
	~WS28xxDmxGrouping() override;

	void Start(uint8_t nPort = 0) override;

	void SetData(uint8_t nPort, const uint8_t *pData, uint16_t nLenght) override;

	void SetLEDType(TWS28XXType tLedType) override;
	void SetLEDCount(uint16_t nLedCount) override;
	void SetLEDGroupCount(uint16_t nLedGroupCount);
	uint32_t GetLEDGroupCount() {
		return m_nLEDGroupCount;
	}

	void Print() override;

public: // RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress) override;
	bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo) override;

private:
	void UpdateMembers();

private:
	uint8_t *m_pDmxData{nullptr};
	uint32_t m_nLEDGroupCount{0};
	uint32_t m_nGroups{0};
};

#endif /* WS28XXDMXGROUPING_H_ */
