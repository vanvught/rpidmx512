/**
 * @file dmxnodeparams.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXNODEPARAMS_H_
#define DMXNODEPARAMS_H_

#include <cstdint>
#include <cassert>

#include "dmxnode.h"
#include "configstore.h"

namespace dmxnodeparams {
struct Params {
   uint32_t nSetList;
   // DmxNode
   uint8_t nPersonality;
   uint16_t nUniverse[dmxnode::PARAM_PORTS];
   uint16_t nDirection;
   uint16_t nMergeMode;
   uint8_t nOutputStyle;
   uint8_t nFailSafe;
   uint8_t aLongName[dmxnode::NODE_NAME_LENGTH];
   uint8_t aLabel[dmxnode::PARAM_PORTS][dmxnode::LABEL_NAME_LENGTH];
   uint8_t Filler1;
   // Art-Net 4
   uint16_t nProtocol;
   uint16_t nRdm;
   uint32_t nDestinationIp[dmxnode::PARAM_PORTS];
   // sACN E1.31
   uint8_t nPriority[dmxnode::PARAM_PORTS];
   // Reserved
   uint8_t Filler2[40];
} __attribute__((packed));

static_assert(sizeof(struct Params) <= configstore::STORE_SIZE[static_cast<uint32_t>(configstore::Store::NODE)], "struct Params is too large");

struct Mask {
	static constexpr uint32_t DISABLE_MERGE_TIMEOUT	= (1U << 11);
	static constexpr uint32_t ENABLE_RDM    		= (1U << 16);
	static constexpr uint32_t MAP_UNIVERSE0 		= (1U << 17);

	static constexpr uint32_t MASK_NODE = DISABLE_MERGE_TIMEOUT;
	static constexpr uint32_t MASK_ARTNET = ENABLE_RDM | MAP_UNIVERSE0;
	static constexpr uint32_t MASK_SACN = 0;
};

template <class S>
static void port_set(const uint32_t nPortIndex, const S s, uint16_t& n) {
	uint16_t value = n; // Create a local copy
	value &= static_cast<uint16_t>(~(0x3 << (nPortIndex * 2)));
	value |= static_cast<uint16_t>((static_cast<uint32_t>(s) & 0x3) << (nPortIndex * 2));
	n = value; // Write back to the original field
}

template <class S>
static S port_get(const uint32_t nPortIndex, const uint16_t n) {
	return static_cast<S>((n >> (nPortIndex * 2)) & 0x3);
}
}  // namespace dmxnodeparams

class DmxNodeParams {
public:
	DmxNodeParams(dmxnode::Personality personality);
	~DmxNodeParams() = default;

	void Load();
	void Load(const char *pBuffer, const uint32_t nLength);

	void Builder(const struct dmxnodeparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set();

	bool IsRdm() const {
		return IsMaskSet(dmxnodeparams::Mask::ENABLE_RDM);
	}

	static void StaticCallbackFunction(void *p, const char *s) {
		assert(p != nullptr);
		assert(s != nullptr);

		(static_cast<DmxNodeParams *>(p))->CallbackFunction(s);
	}

private:
	void CallbackFunction(const char *);
	void Dump();

	bool IsMaskSet(const uint32_t nMask) const {
		return (m_Params.nSetList & nMask) == nMask;
	}

	bool IsOutputStyleSet(const uint8_t nMask) const {
		return (m_Params.nOutputStyle & nMask) == nMask;
	}

	void SetFactoryDefaults();

private:
	dmxnode::Personality m_Personality;
	dmxnodeparams::Params m_Params;
};

#endif /* DMXNODEPARAMS_H_ */
