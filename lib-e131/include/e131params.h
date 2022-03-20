/**
 * @file e131params.h
 *
 */
/* Copyright (C) 2016-2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef E131PARAMS_H_
#define E131PARAMS_H_

#include <cstdint>

#include "e131bridge.h"
#include "lightset.h"

namespace e131params {
	constexpr auto MAX_PORTS = 4;

struct Params {
    uint32_t nSetList;
    uint8_t nOutputType;
#if defined (OUTPUT_DMX_ARTNET)
    uint16_t nUniverse;
#else
	uint16_t NotUsed0;
#endif
    uint16_t nUniversePort[e131params::MAX_PORTS];
#if defined (OUTPUT_DMX_ARTNET)
	uint8_t nMergeMode;
#else
	uint8_t NotUsed1;
#endif
	uint8_t nMergeModePort[e131params::MAX_PORTS];
	uint32_t NotUsed2;
	uint8_t NotUsed3;
	uint8_t NotUsed4;
	uint8_t nDirection;
	uint8_t nPriority[e131params::MAX_PORTS];
} __attribute__((packed));

static_assert(sizeof(struct Params) <= 96, "struct Params is too large");

struct Mask {
	static constexpr auto UNIVERSE = (1U << 0);
	static constexpr auto MERGE_MODE = (1U << 1);
	static constexpr auto OUTPUT = (1U << 2);
	//static constexpr auto NOT_USED = (1U << 3);
	static constexpr auto UNIVERSE_A = (1U << 4);
	static constexpr auto UNIVERSE_B = (1U << 5);
	static constexpr auto UNIVERSE_C = (1U << 6);
	static constexpr auto UNIVERSE_D = (1U << 7);
	static constexpr auto MERGE_MODE_A = (1U << 8);
	static constexpr auto MERGE_MODE_B = (1U << 9);
	static constexpr auto MERGE_MODE_C = (1U << 10);
	static constexpr auto MERGE_MODE_D = (1U << 11);
	static constexpr auto DISABLE_NETWORK_DATA_LOSS_TIMEOUT = (1U << 12);
	static constexpr auto DISABLE_MERGE_TIMEOUT = (1U << 13);
	//static constexpr auto NOT_USED1 = (1U << 14);
	static constexpr auto DIRECTION = (1U << 15);
	static constexpr auto PRIORITY_A = (1U << 16);
	static constexpr auto PRIORITY_B = (1U << 17);
	static constexpr auto PRIORITY_C = (1U << 18);
	static constexpr auto PRIORITY_D = (1U << 19);
};
}

class E131ParamsStore {
public:
	virtual ~E131ParamsStore() {}

	virtual void Update(const struct e131params::Params *pParams)=0;
	virtual void Copy(struct e131params::Params *pParams)=0;
};

class E131Params {
public:
	E131Params(E131ParamsStore *pE131ParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct e131params::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(E131Bridge *);

	void Dump();

	lightset::OutputType GetOutputType() const {
		return static_cast<lightset::OutputType>(m_Params.nOutputType);
	}
	
#if defined (OUTPUT_DMX_ARTNET)
	uint16_t GetUniverse() const {
		return m_Params.nUniverse;
	}
#endif

	uint16_t GetUniverse(uint32_t nPortIndex, bool &IsSet) const {
		if (nPortIndex < e131params::MAX_PORTS) {
			IsSet = isMaskSet(e131params::Mask::UNIVERSE_A << nPortIndex);
			return m_Params.nUniversePort[nPortIndex];
		}
		IsSet = false;
		return 0;
	}

	lightset::PortDir GetDirection(uint32_t nPortIndex) const {
		if (nPortIndex < e131params::MAX_PORTS) {
			return static_cast<lightset::PortDir>((m_Params.nDirection >> nPortIndex) & 0x1);
		}
		return lightset::PortDir::DISABLE;
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }

private:
    E131ParamsStore *m_pE131ParamsStore;
    e131params::Params m_Params;
};

#endif /* E131PARAMS_H_ */
