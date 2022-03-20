/**
 * @file dmxparams.h
 *
 */
/* Copyright (C) 2017-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DMXPARAMS_H_
#define DMXPARAMS_H_

#include <cstdint>

#include "dmx.h"

struct TDmxParams {
    uint32_t nSetList;
	uint16_t nBreakTime;
	uint16_t nMabTime;
	uint8_t nRefreshRate;
	uint8_t nSlotsCount;
}__attribute__((packed));

static_assert(sizeof(struct TDmxParams) <= 32, "struct TDmxParams is too large");

struct DmxParamsMask {
	static constexpr auto BREAK_TIME = (1U << 0);
	static constexpr auto MAB_TIME = (1U << 1);
	static constexpr auto REFRESH_RATE = (1U << 2);
	static constexpr auto SLOTS_COUNT = (1U << 3);
};

namespace dmxparams {
static constexpr uint8_t rounddown_slots(uint16_t n) {
	return static_cast<uint8_t>((n / 2U) - 1);
}
static constexpr uint16_t roundup_slots(uint8_t n) {
	return static_cast<uint16_t>((n + 1U) * 2U);
}
}  // namespace dmxparams

class DmxParamsStore {
public:
	virtual ~DmxParamsStore() {}

	virtual void Update(const struct TDmxParams *pDmxParams)=0;
	virtual void Copy(struct TDmxParams *pDmxParams)=0;
};

class DmxParams {
public:
	DmxParams(DmxParamsStore *pDMXParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TDmxParams *pTDmxParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(Dmx *);

	void Dump();

    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const  {
    	return (m_tDmxParams.nSetList & nMask) == nMask;
    }

private:
    DmxParamsStore *m_pDmxParamsStore;
    struct TDmxParams m_tDmxParams;
};

#endif /* DMXPARAMS_H_ */
