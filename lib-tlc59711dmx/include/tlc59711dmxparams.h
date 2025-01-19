/**
 * @file tlc59711dmxparams.h
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

#ifndef TLC59711DMXPARAMS_H_
#define TLC59711DMXPARAMS_H_

#include <cstdint>

#include "tlc59711dmx.h"
#include "configstore.h"

namespace tlc59711dmxparams {
struct Params {
    uint32_t nSetList;
	uint8_t nType;
	uint8_t nCount;
	uint16_t nDmxStartAddress;
    uint32_t nSpiSpeedHz;
} __attribute__((packed));

static_assert(sizeof(struct Params) <= 64, "struct Params is too large");

struct Mask {
	static constexpr auto TYPE = (1U << 0);
	static constexpr auto COUNT = (1U << 1);
	static constexpr auto DMX_START_ADDRESS = (1U << 2);
	static constexpr auto SPI_SPEED = (1U << 3);
};
}  // namespace tlc59711dmxparams

class TLC59711DmxParamsStore {
public:
	static void Update(const struct tlc59711dmxparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::TLC5711DMX, pParams, sizeof(struct tlc59711dmxparams::Params));
	}

	static void Copy(struct tlc59711dmxparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::TLC5711DMX, pParams, sizeof(struct tlc59711dmxparams::Params));
	}
};

class TLC59711DmxParams {
public:
	TLC59711DmxParams();

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct tlc59711dmxparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set(TLC59711Dmx *);

	tlc59711::Type GetLedType() {
		return static_cast<tlc59711::Type>(m_Params.nType);
	}

	uint16_t GetLedCount() {
		return m_Params.nCount;
	}

	bool IsSetLedType() {
		return isMaskSet(tlc59711dmxparams::Mask::TYPE);
	}

	bool IsSetLedCount() {
		return isMaskSet(tlc59711dmxparams::Mask::COUNT);
	}

	static const char *GetType(tlc59711::Type type);
	static tlc59711::Type GetType(const char *pValue);
    static void StaticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *pLine);
    bool isMaskSet(uint32_t nMask) {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
	tlc59711dmxparams::Params m_Params;
};

#endif /* TLC59711DMXPARAMS_H_ */
