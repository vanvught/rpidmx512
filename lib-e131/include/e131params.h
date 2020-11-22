/**
 * @file e131params.h
 *
 */
/* Copyright (C) 2016-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <uuid/uuid.h>

#include "e131bridge.h"

#include "lightset.h"

namespace E131_PARAMS {
	constexpr auto MAX_PORTS = 4;
}

struct TE131Params {
    uint32_t nSetList;
    TLightSetOutputType tOutputType;
    uint16_t nUniverse;
    uint16_t nUniversePort[E131_PARAMS::MAX_PORTS];
	uint8_t nMergeMode;
	uint8_t nMergeModePort[E131_PARAMS::MAX_PORTS];
	float nNetworkTimeout;
	bool bDisableMergeTimeout;
	bool bEnableNoChangeUpdate;
	uint8_t nDirection;
	uint8_t nPriority;
};
//} __attribute__((packed));

static_assert(sizeof(struct TE131Params) <= 96, "struct TE131Params is too large");

struct E131ParamsMask {
	static constexpr auto UNIVERSE = (1U << 0);
	static constexpr auto MERGE_MODE = (1U << 1);
	static constexpr auto OUTPUT = (1U << 2);
	static constexpr auto CID = (1U << 3);
	static constexpr auto UNIVERSE_A = (1U << 4);
	static constexpr auto UNIVERSE_B = (1U << 5);
	static constexpr auto UNIVERSE_C = (1U << 6);
	static constexpr auto UNIVERSE_D = (1U << 7);
	static constexpr auto MERGE_MODE_A = (1U << 8);
	static constexpr auto MERGE_MODE_B = (1U << 9);
	static constexpr auto MERGE_MODE_C = (1U << 10);
	static constexpr auto MERGE_MODE_D = (1U << 11);
	static constexpr auto NETWORK_TIMEOUT = (1U << 12);
	static constexpr auto MERGE_TIMEOUT = (1U << 13);
	static constexpr auto ENABLE_NO_CHANGE_OUTPUT = (1U << 14);
	static constexpr auto DIRECTION = (1U << 15);
	static constexpr auto PRIORITY = (1U << 16);
};

class E131ParamsStore {
public:
	virtual ~E131ParamsStore() {
	}

	virtual void Update(const struct TE131Params *pE131Params)=0;
	virtual void Copy(struct TE131Params *pE131Params)=0;
};

class E131Params {
public:
	E131Params(E131ParamsStore *pE131ParamsStore = nullptr);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct TE131Params *ptE131Params, char *pBuffer, uint32_t nLength, uint32_t &nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t &nSize);

	void Set(E131Bridge *);

	void Dump();

	TLightSetOutputType GetOutputType() const {
		return m_tE131Params.tOutputType;
	}

	uint16_t GetUniverse() const {
		return m_tE131Params.nUniverse;
	}

	E131Merge GetMergeMode() const {
		return static_cast<E131Merge>(m_tE131Params.nMergeMode);
	}

	uint16_t GetUniverse(uint8_t nPort, bool &IsSet);

	bool IsEnableNoChangeUpdate() const {
		return m_tE131Params.bEnableNoChangeUpdate;
	}

	TE131PortDir GetDirection() const {
		return static_cast<TE131PortDir>(m_tE131Params.nDirection);
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
    bool isMaskSet(uint32_t nMask) const {
    	return (m_tE131Params.nSetList & nMask) == nMask;
    }

private:
    E131ParamsStore *m_pE131ParamsStore;
    struct TE131Params m_tE131Params;
    uuid_t m_uuid;
};

#endif /* E131PARAMS_H_ */
