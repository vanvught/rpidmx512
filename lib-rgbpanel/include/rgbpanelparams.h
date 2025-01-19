/**
 * @file rgbpanelparams.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>

#include "rgbpanelconst.h"
#include "configstore.h"

namespace rgbpanelparams {
struct Params {
	uint32_t nSetList;
	uint8_t nCols;
	uint8_t nRows;
	uint8_t nChain;
	uint8_t nType;
} __attribute__((packed));

static_assert(sizeof(struct Params) <= 32, "struct Params is too large");

struct Mask {
	static constexpr auto COLS = (1U << 0);
	static constexpr auto ROWS = (1U << 1);
	static constexpr auto CHAIN = (1U << 2);
	static constexpr auto TYPE = (1U << 3);
};
}  // namespace rgbpanelparams

class RgbPanelParamsStore {
public:
	static void Update(const struct rgbpanelparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::RGBPANEL, pParams, sizeof(struct rgbpanelparams::Params));
	}

	static void Copy(struct rgbpanelparams::Params *pRgbPanelParamss) {
		ConfigStore::Get()->Copy(configstore::Store::RGBPANEL, pRgbPanelParamss, sizeof(struct rgbpanelparams::Params));
	}
};

class RgbPanelParams {
public:
	RgbPanelParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct rgbpanelparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	uint32_t GetCols() const {
		return m_Params.nCols;
	}

	uint32_t GetRows() const {
		return m_Params.nRows;
	}

	uint32_t GetChain() const {
		return m_Params.nChain;
	}

	rgbpanel::Types GetType() const {
		return static_cast<rgbpanel::Types>(m_Params.nType);
	}

    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
	void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const {
		return (m_Params.nSetList & nMask) == nMask;
	}

private:
	rgbpanelparams::Params m_Params;
};

#endif /* RGBPANELPARAMS_H_ */
