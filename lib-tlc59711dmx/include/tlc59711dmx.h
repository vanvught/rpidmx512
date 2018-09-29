/**
 * @file tlc59711dmx.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

enum TTLC59711Type {
	TTLC59711_TYPE_RGB,
	TTLC59711_TYPE_RGBW
};

class TLC59711Dmx: public LightSet {
public:
	TLC59711Dmx(void);
	~TLC59711Dmx(void);

	void Start(uint8_t nPort = 0);
	void Stop(uint8_t nPort = 0);

	void SetData(uint8_t nPort, const uint8_t *pDmxData, uint16_t nLength);

	void SetLEDType(TTLC59711Type tTLC59711Type);
	TTLC59711Type GetLEDType(void) const;

	void SetLEDCount(uint8_t nLEDCount);
	uint8_t GetLEDCount(void) const;

	void SetSpiSpeedHz(uint32_t nSpiSpeedHz);
	uint32_t GetSpiSpeedHz(void) const;


public: // RDM
	bool SetDmxStartAddress(uint16_t nDmxStartAddress);

	inline uint16_t GetDmxStartAddress(void) {
		return m_nDmxStartAddress;
	}

	inline uint16_t GetDmxFootprint(void) {
		return m_nDmxFootprint;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, struct TLightSetSlotInfo &tSlotInfo);

public:
	inline uint8_t GetBoardInstances(void) {
		return m_nBoardInstances;
	}

private:
	void Initialize(void);
	void UpdateMembers(void);

private:
	uint16_t m_nDmxStartAddress;
	uint16_t m_nDmxFootprint;
	uint8_t m_nBoardInstances;
	bool m_bIsStarted;
	TLC59711 *m_pTLC59711;
	uint32_t m_nSpiSpeedHz;
	TTLC59711Type m_LEDType;
	uint8_t m_nLEDCount;
};

#endif /* TLC59711DMX_H_ */
