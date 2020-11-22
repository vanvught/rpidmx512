/**
 * @file tlc59711dmx.h
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>

#include "lightset.h"

#include "tlc59711.h"
#include "tlc59711dmxstore.h"

enum TTLC59711Type {
	TTLC59711_TYPE_RGB,
	TTLC59711_TYPE_RGBW,
	TTLC59711_TYPE_UNDEFINED
};

class TLC59711Dmx final: public LightSet {
public:
	TLC59711Dmx();
	~TLC59711Dmx() override;

	void Start(uint8_t nPort = 0) override;
	void Stop(uint8_t nPort = 0) override;

	void SetData(uint8_t nPort, const uint8_t *pDmxData, uint16_t nLength) override;

	void Blackout(bool bBlackout);

	void SetLEDType(TTLC59711Type tTLC59711Type);
	TTLC59711Type GetLEDType() {
		return m_LEDType;
	}

	void SetLEDCount(uint8_t nLEDCount);
	uint8_t GetLEDCount() {
		return m_nLEDCount;
	}

	void SetSpiSpeedHz(uint32_t nSpiSpeedHz);
	uint32_t GetSpiSpeedHz() {
		return m_nSpiSpeedHz;
	}

	void SetTLC59711DmxStore(TLC59711DmxStore *pTLC59711Store) {
		m_pTLC59711DmxStore = pTLC59711Store;
	}

	void Print() override;

public: // RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress) override;

	inline uint16_t GetDmxStartAddress() override {
		return m_nDmxStartAddress;
	}

	inline uint16_t GetDmxFootprint() override {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo) override;

public:
	inline uint8_t GetBoardInstances() {
		return m_nBoardInstances;
	}

private:
	void Initialize();
	void UpdateMembers();

private:
	uint16_t m_nDmxStartAddress{1};
	uint16_t m_nDmxFootprint;
	uint8_t m_nBoardInstances{1};
	bool m_bIsStarted{false};
	bool m_bBlackout{false};
	TLC59711 *m_pTLC59711{nullptr};
	uint32_t m_nSpiSpeedHz{0};
	TTLC59711Type m_LEDType{TTLC59711_TYPE_RGB};
	uint8_t m_nLEDCount;

	TLC59711DmxStore *m_pTLC59711DmxStore{nullptr};
};

#endif /* TLC59711DMX_H_ */
