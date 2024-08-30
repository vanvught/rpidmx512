/**
 * @file pixeldmxparams.h
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"); to deal
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

#ifndef PIXELDMXPARAMS_H_
#define PIXELDMXPARAMS_H_

#include <cstdint>

#include "pixeldmxconfiguration.h"
#include "configstore.h"

#if !defined (CONFIG_PIXELDMX_MAX_PORTS)
# error CONFIG_PIXELDMX_MAX_PORTS is not defined
#endif

namespace pixeldmxparams {
static constexpr auto MAX_PORTS = CONFIG_PIXELDMX_MAX_PORTS;

struct Params {
    uint32_t nSetList;										///< 4	   4
	uint8_t nType;											///< 1	   5
	uint16_t nCount;										///< 2	   7
	uint16_t nDmxStartAddress;								///< 2	   9
	uint8_t nGammaValue;									///< 1	  10
	uint32_t nSpiSpeedHz;									///< 4	  14
	uint8_t nGlobalBrightness;								///< 1	  15
	uint8_t nActiveOutputs;									///< 1	  16
	uint8_t nTestPattern;									///< 1	  17
	uint16_t nGroupingCount;								///< 2	  19
	uint8_t nMap;											///< 1	  20
	uint8_t nLowCode;										///< 1	  21
	uint8_t nHighCode;										///< 1	  22
	uint16_t nStartUniverse[pixeldmxparams::MAX_PORTS];		///< 16   38
}__attribute__((packed));

static_assert(sizeof(struct Params) <= 64, "struct Params is too large");

struct Mask {
	static constexpr auto TYPE = (1U << 0);
	static constexpr auto COUNT = (1U << 1);
	static constexpr auto DMX_START_ADDRESS = (1U << 2);
	static constexpr auto TEST_PATTERN = (1U << 3);
	static constexpr auto SPI_SPEED = (1U << 4);
	static constexpr auto GLOBAL_BRIGHTNESS = (1U << 5);
	static constexpr auto ACTIVE_OUT = (1U << 6);
	static constexpr auto GAMMA_CORRECTION = (1U << 7);
	static constexpr auto GROUPING_COUNT = (1U << 8);
	static constexpr auto MAP = (1U << 9);
	static constexpr auto LOW_CODE = (1U << 10);
	static constexpr auto HIGH_CODE = (1U << 11);
	static constexpr auto START_UNI_PORT_1 = (1U << 12);
};
}  // pixeldmxparams

class PixelDmxParamsStore {
public:
	static void Update(const struct pixeldmxparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::WS28XXDMX, pParams, sizeof(struct pixeldmxparams::Params));
	}

	static void Copy(struct pixeldmxparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::WS28XXDMX, pParams, sizeof(struct pixeldmxparams::Params));
	}
};

class PixelDmxParams {
public:
	PixelDmxParams();
	~PixelDmxParams() = default;

	void Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct pixeldmxparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set();

	uint16_t GetStartUniversePort(uint32_t nOutputPortIndex, bool& isSet) const {
		if (nOutputPortIndex < pixeldmxparams::MAX_PORTS) {
			isSet = isMaskSet(pixeldmxparams::Mask::START_UNI_PORT_1 << nOutputPortIndex);
			return m_Params.nStartUniverse[nOutputPortIndex];
		}

		isSet = false;
		return 0;
	}

	uint8_t GetTestPattern() const {
		return m_Params.nTestPattern;
	}

	static void staticCallbackFunction(void *p, const char *s);

private:
	void Dump();
    void callbackFunction(const char *pLine);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    pixeldmxparams::Params m_Params;
};

#endif /* PIXELDMXPARAMS_H_ */
