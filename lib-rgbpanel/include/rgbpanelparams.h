/**
 * @file rgbpanelparams.h
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

#ifndef RGBPANELPARAMS_H_
#define RGBPANELPARAMS_H_

#include <stdint.h>

#include "rgbpanelconst.h"

struct TRgbPanelParams {
	uint32_t nSetList;
	uint8_t nCols;
	uint8_t nRows;
	uint8_t nChain;
	uint8_t nType;
} __attribute__((packed));

static_assert(sizeof(struct TRgbPanelParams) <= 32, "struct TRgbPanelParams is too large");

struct RgbPanelParamsMask {
	static constexpr auto COLS = (1U << 0);
	static constexpr auto ROWS = (1U << 1);
	static constexpr auto CHAIN = (1U << 2);
	static constexpr auto TYPE = (1U << 3);
};

class RgbPanelParamsStore {
public:
	virtual ~RgbPanelParamsStore() {
	}

	virtual void Update(const struct TRgbPanelParams *pRgbPanelParams)=0;
	virtual void Copy(struct TRgbPanelParams *pRgbPanelParams)=0;
};

class RgbPanelParams {
public:
	RgbPanelParams(RgbPanelParamsStore *pRgbPanelParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TRgbPanelParams *pRgbPanelParams, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Dump();

	uint32_t GetCols() const {
		return m_tRgbPanelParams.nCols;
	}

	uint32_t GetRows() const {
		return m_tRgbPanelParams.nRows;
	}

	uint32_t GetChain() const {
		return m_tRgbPanelParams.nChain;
	}

	rgbpanel::Types GetType() const {
		return static_cast<rgbpanel::Types>(m_tRgbPanelParams.nType);
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
	void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const {
		return (m_tRgbPanelParams.nSetList & nMask) == nMask;
	}

private:
	RgbPanelParamsStore *m_pRgbPanelParamsStore;
	struct TRgbPanelParams m_tRgbPanelParams;
};

#endif /* RGBPANELPARAMS_H_ */
