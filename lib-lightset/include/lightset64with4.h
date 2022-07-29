/**
 * @file lightset64with4.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LIGHTSET64WITH4_H_
#define LIGHTSET64WITH4_H_

#include "lightset.h"

#include "debug.h"

class LightSet64with4 final: public LightSet {
public:
	LightSet64with4(LightSet *pA, LightSet *pB) : m_pA(pA), m_pB(pB) {
		DEBUG_PRINTF("%p %p", m_pA, m_pB);
	}
	~LightSet64with4() override {}

	void SetLightSetA(LightSet *pA) {
		m_pA = pA;
	}

	LightSet *GetLightSetA() const {
		return m_pA;
	}

	void SetLightSetB(LightSet *pB) {
		m_pB = pB;
	}

	LightSet *GetLightSetB() const {
		return m_pB;
	}

	void Start(uint32_t nPortIndex) override {
		if ((nPortIndex < 32) && (m_pA != nullptr)) {
			return m_pA->Start(nPortIndex);
		}
		if (m_pB != nullptr) {
			m_pB->Start(nPortIndex & 0x3);
		}
	}

	void Stop(uint32_t nPortIndex) override {
		if ((nPortIndex < 64) && (m_pA != nullptr)) {
			return m_pA->Stop(nPortIndex);
		}
		if (m_pB != nullptr) {
			m_pB->Stop(nPortIndex & 0x3);
		}
	}

	void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) override {
		if ((nPortIndex < 64) && (m_pA != nullptr)) {
			return m_pA->SetData(nPortIndex, pData, nLength);
		}
		if (m_pB != nullptr) {
			return m_pB->SetData(nPortIndex & 0x3, pData, nLength);
		}
	}

	void Blackout(bool bBlackout) override {
		if (m_pA != nullptr) {
			m_pA->Blackout(bBlackout);
		}
		if (m_pB != nullptr) {
			m_pB->Blackout(bBlackout);
		}
	}

	void Print() override {
		if (m_pA != nullptr) {
			m_pA->Print();
		}
		if (m_pB != nullptr) {
			m_pB->Print();
		}
	}

	bool SetDmxStartAddress(__attribute__((unused)) uint16_t nDmxStartAddress) override {
		return false;
	}

	uint16_t GetDmxStartAddress() override {
		return lightset::dmx::ADDRESS_INVALID;
	}

	uint16_t GetDmxFootprint() override {
		return 0;
	}

	bool GetSlotInfo(__attribute__((unused)) uint16_t nSlotOffset, __attribute__((unused)) lightset::SlotInfo &tSlotInfo) override {
		return false;
	}

private:
	LightSet *m_pA;
	LightSet *m_pB;
};

#endif /* LIGHTSET64WITH4_H_ */
