/**
 * @file gpsparams.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef GPSPARAMS_H_
#define GPSPARAMS_H_

#include <stdint.h>

#include "gpsconst.h"

struct TGPSParams {
	uint32_t nSetList;
	uint8_t nModule;
	uint8_t nEnable;
	float fUtcOffset;
} __attribute__((packed));

struct GPSParamsMask {
	static constexpr auto MODULE = (1U << 0);
	static constexpr auto ENABLE = (1U << 1);
	static constexpr auto UTC_OFFSET = (1U << 2);
};

class GPSParamsStore {
public:
	virtual ~GPSParamsStore() {
	}

	virtual void Update(const struct TGPSParams *pGPSParams)=0;
	virtual void Copy(struct TGPSParams *pGPSParams)=0;
};

class GPSParams {
public:
	GPSParams(GPSParamsStore *pGPSParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TGPSParams *pGPSParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Dump();

	bool IsEnabled() const {
		return (m_tTGPSParams.nEnable == 1);
	}

	GPSModule GetModule() const {
		return static_cast<GPSModule>(m_tTGPSParams.nModule);
	}

	float GetUtcOffset() const {
		return m_tTGPSParams.fUtcOffset;
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
	void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const {
		return (m_tTGPSParams.nSetList & nMask) == nMask;
	}

private:
	GPSParamsStore *m_pGPSParamsStore;
	struct TGPSParams m_tTGPSParams;
};

#endif /* GPSPARAMS_H_ */
