/**
 * @file dmxparams.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include "configstore.h"

namespace dmxsendparams {
struct Params {
    uint32_t nSetList;
	uint16_t nBreakTime;
	uint16_t nMabTime;
	uint8_t nRefreshRate;
	uint8_t nSlotsCount;
}__attribute__((packed));

static_assert(sizeof(struct Params) <= 32, "struct Params is too large");

struct Mask {
	static constexpr uint32_t BREAK_TIME = (1U << 0);
	static constexpr uint32_t MAB_TIME = (1U << 1);
	static constexpr uint32_t REFRESH_RATE = (1U << 2);
	static constexpr uint32_t SLOTS_COUNT = (1U << 3);
};

static constexpr uint8_t rounddown_slots(uint16_t n) {
	return static_cast<uint8_t>((n / 2U) - 1);
}
static constexpr uint16_t roundup_slots(uint8_t n) {
	return static_cast<uint16_t>((n + 1U) * 2U);
}
}  // namespace dmxsendparams]

class StoreDmxSend {
public:
	static void Update(const struct dmxsendparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::DMXSEND, pParams, sizeof(struct dmxsendparams::Params));
	}

	static void Copy(struct dmxsendparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::DMXSEND, pParams, sizeof(struct dmxsendparams::Params));
	}
};

class DmxParams {
public:
	DmxParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct dmxsendparams::Params *pTDmxParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(Dmx *);

    static void staticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const  {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    dmxsendparams::Params m_Params;
};

#endif /* DMXPARAMS_H_ */
