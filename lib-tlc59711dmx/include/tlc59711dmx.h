/**
 * @file tlc59711dmx.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef TLC59711DMX_H_
#define TLC59711DMX_H_

#include <cstdint>

#include "tlc59711.h"
#include "dmxnode.h"

namespace tlc59711 {
enum class Type {
	RGB, RGBW, UNDEFINED
};
}  // namespace tlc59711

class TLC59711Dmx {
public:
	TLC59711Dmx();
	~TLC59711Dmx() ;

	void Start(uint32_t nPortIndex = 0) ;
	void Stop(uint32_t nPortIndex = 0) ;

	void SetData(const uint32_t nPortIndex, const uint8_t *pDmxData, uint32_t nLength, const bool doUpdate = true) ;
	void Sync(const uint32_t nPortIndex) ;
	void Sync() ;

#if defined (CONFIG_TLC59711DMX_ENABLE_PCT)
	void SetMaxPct(uint32_t nIndexLed, uint32_t nPct);
#endif
	void Blackout(bool bBlackout) ;

	void Print() ;

	void SetType(tlc59711::Type type);
	tlc59711::Type GetType() {
		return m_type;
	}

	void SetCount(uint32_t nCount);
	uint16_t GetCount() const {
		return m_nCount;
	}

	void SetSpiSpeedHz(uint32_t nSpiSpeedHz);
	uint32_t GetSpiSpeedHz() const {
		return m_nSpiSpeedHz;
	}

public: // RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress) ;

	uint16_t GetDmxStartAddress()  {
		return m_nDmxStartAddress;
	}

	uint16_t GetDmxFootprint()  {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, dmxnode::SlotInfo &tSlotInfo) ;

	static TLC59711Dmx *Get() {
		return s_pThis;
	}

private:
	void Initialize();
	void UpdateMembers();

private:
	uint16_t m_nDmxFootprint { TLC59711Channels::OUT };
	uint16_t m_nCount { TLC59711Channels::RGB };
	uint32_t m_nSpiSpeedHz { 0 };
	uint16_t m_nDmxStartAddress { 1 };
	bool m_bIsStarted { false };
	bool m_bBlackout { false };
	TLC59711 *m_pTLC59711 { nullptr };
#if defined (CONFIG_TLC59711DMX_ENABLE_PCT)
	uint16_t *m_ArrayMaxValue { nullptr };
#endif
	tlc59711::Type m_type { tlc59711::Type::RGB };

	static TLC59711Dmx *s_pThis;
};

#endif /* TLC59711DMX_H_ */
