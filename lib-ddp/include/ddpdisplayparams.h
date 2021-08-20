/**
 * @file ddpdisplayparams.h
 *
 */
/* Copyright (C) 2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DDPDISPLAYPARAMS_H_
#define DDPDISPLAYPARAMS_H_

#include <ddpdisplaypixelconfiguration.h>
#include <stdint.h>


namespace ddpdisplayparams {
static constexpr auto MAX_PORTS = 8;
static constexpr auto MAX_COUNT = (4 * 170);
}  // namespace ddpdisplayparams

struct TDdpDisplayParams {
    uint32_t nSetList;		///< 4	  4
	uint8_t nType;			///< 1	  5
	uint8_t nMap;			///< 1	  6
	uint8_t nLowCode;		///< 1	  7
	uint8_t nHighCode;		///< 1	  8
	uint16_t nCount[ddpdisplayparams::MAX_PORTS];	///< 9   25
	uint8_t nTestPattern;	///< 1	 26
};

static_assert(sizeof(struct TDdpDisplayParams) <= 32, "struct TDdpDisplayParams is too large");

struct DdpDisplayParamsMask {
	static constexpr auto TYPE = (1U << 0);
	static constexpr auto MAP = (1U << 1);
	static constexpr auto LOW_CODE = (1U << 2);
	static constexpr auto HIGH_CODE = (1U << 3);
	static constexpr auto COUNT_A = (1U << 4);
	static constexpr auto TEST_PATTERN = (1U << 31);
};

class DdpDisplayParamsStore {
public:
	virtual ~DdpDisplayParamsStore() {}

	virtual void Update(const struct TDdpDisplayParams *pDdpDisplayParams)=0;
	virtual void Copy(struct TDdpDisplayParams *pDdpDisplayParams)=0;
};

class DdpDisplayParams {
public:
	DdpDisplayParams(DdpDisplayParamsStore *pDdpDisplayParamsStore = nullptr);
	~DdpDisplayParams() {}

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TDdpDisplayParams *pDdpDisplayParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(DdpDisplayPixelConfiguration *pDdpPixelConfiguration);

	void Dump();

	uint8_t GetTestPattern() const {
		return m_tTDdpDisplayParams.nTestPattern;
	}

	static void staticCallbackFunction(void *p, const char *s);

private:
	void callbackFunction(const char *pLine);
	bool isMaskSet(uint32_t nMask) const {
		return (m_tTDdpDisplayParams.nSetList & nMask) == nMask;
	}

private:
	DdpDisplayParamsStore *m_pDdpDisplayParamsStore;
	struct TDdpDisplayParams m_tTDdpDisplayParams;
};

#endif /* DDPDISPLAYPARAMS_H_ */
