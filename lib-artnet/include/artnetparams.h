/**
 * @file artnetparams.h
 *
 */
/**
 * Art-Net Designed by and Copyright Artistic Licence Holdings Ltd.
 */
/* Copyright (C) 2016-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETPARAMS_H_
#define ARTNETPARAMS_H_

#include <cstdint>
#include <climits>
#include <cassert>

#include "artnet.h"
#include "configstore.h"
#include "lightset.h"

namespace artnetparams {
static constexpr uint16_t clear_mask(const uint32_t i) {
   	return static_cast<uint16_t>(~((1U << (i + 8)) | (1U << i)));
}

static constexpr uint16_t shift_left(const uint32_t nValue, const uint32_t i) {
	return static_cast<uint16_t>((nValue & 0x1) << i);
}

static constexpr uint16_t portdir_clear(const uint32_t i) {
  	return static_cast<uint16_t>(~(0x3 << (i * 2)));
}

static constexpr uint16_t mergemode_clear(const uint32_t i) {
   	return static_cast<uint16_t>(~(0x3 << (i * 2)));
}

static constexpr uint16_t mergemode_set(const uint32_t nPortIndex, const lightset::MergeMode mergeMode) {
	return static_cast<uint16_t>((static_cast<uint32_t>(mergeMode) & 0x3) << (nPortIndex * 2));
}

static constexpr uint16_t protocol_clear(const uint32_t i) {
   	return static_cast<uint16_t>(~(0x3 << (i * 2)));
}

static constexpr uint16_t protocol_set(const uint32_t nPortIndex, const artnet::PortProtocol protocol) {
	return static_cast<uint16_t>((static_cast<uint32_t>(protocol) & 0x3) << (nPortIndex * 2));
}

struct Params {
   uint32_t nSetList;
   // Node
   uint8_t nPersonality;
   uint16_t nUniverse[artnet::PORTS];
   uint16_t nDirection;
   uint16_t nMergeMode;
   uint8_t nOutputStyle;
   uint8_t nFailSafe;
   uint8_t aLongName[artnet::LONG_NAME_LENGTH];
   uint8_t aLabel[artnet::PORTS][artnet::SHORT_NAME_LENGTH];
   uint8_t Filler1;
   // Art-Net 4
   uint16_t nProtocol;
   uint16_t nRdm;
   uint32_t nDestinationIp[artnet::PORTS];
   // sACN E1.31
   uint8_t nPriority[artnet::PORTS];
   // Reserved
   uint8_t Filler2[40];
} __attribute__((packed));

static_assert(sizeof(struct Params) <= 320, "struct Params is too large");

struct Mask {
	// Node
	//static constexpr uint32_t   					= (1U << 0);
	static constexpr uint32_t UNIVERSE_A    		= (1U << 1);
	static constexpr uint32_t UNIVERSE_B    		= (1U << 2);
	static constexpr uint32_t UNIVERSE_C    		= (1U << 3);
	static constexpr uint32_t UNIVERSE_D    		= (1U << 4);
	static constexpr uint32_t FAILSAFE   			= (1U << 5);
	static constexpr uint32_t LONG_NAME   			= (1U << 6);
	static constexpr uint32_t LABEL_A   			= (1U << 7);
	static constexpr uint32_t LABEL_B   			= (1U << 8);
	static constexpr uint32_t LABEL_C   			= (1U << 9);
	static constexpr uint32_t LABEL_D   			= (1U << 10);
	static constexpr uint32_t DISABLE_MERGE_TIMEOUT	= (1U << 11);
	// Art-Net 4
	static constexpr uint32_t ENABLE_RDM    		= (1U << 16);
	static constexpr uint32_t MAP_UNIVERSE0 		= (1U << 17);
	static constexpr uint32_t DESTINATION_IP_A    	= (1U << 18);
	static constexpr uint32_t DESTINATION_IP_B    	= (1U << 19);
	static constexpr uint32_t DESTINATION_IP_C    	= (1U << 20);
	static constexpr uint32_t DESTINATION_IP_D    	= (1U << 21);
	// sACN E1.31
	static constexpr uint32_t PRIORITY_A    		= (1U << 22);
	static constexpr uint32_t PRIORITY_B    		= (1U << 22);
	static constexpr uint32_t PRIORITY_C    		= (1U << 23);
	static constexpr uint32_t PRIORITY_D    		= (1U << 24);
};

}  // namespace artnetparams

class ArtNetParamsStore {
public:
	static void Update(const struct artnetparams::Params *pParams) {
		ConfigStore::Get()->Update(configstore::Store::NODE, pParams, sizeof(struct artnetparams::Params));
	}

	static void Copy(struct artnetparams::Params *pParams) {
		ConfigStore::Get()->Copy(configstore::Store::NODE, pParams, sizeof(struct artnetparams::Params));
	}
};

class ArtNetParams {
public:
	ArtNetParams();

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct artnetparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
		Builder(nullptr, pBuffer, nLength, nSize);
	}

	void Set();

	bool IsRdm() const {
		return isMaskSet(artnetparams::Mask::ENABLE_RDM);
	}

	uint16_t GetUniverse(uint32_t nPortIndex) const {
		if (nPortIndex < artnet::PORTS) {
			return m_Params.nUniverse[nPortIndex];
		}
		return UINT16_MAX;
	}

	lightset::PortDir GetDirection(uint32_t nPortIndex) const {
		if (nPortIndex < artnet::PORTS) {
			return portdir_get(nPortIndex);
		}
		return lightset::PortDir::DISABLE;
	}

	static void staticCallbackFunction(void *p, const char *s);

private:
	lightset::PortDir portdir_get(const uint32_t nPortIndex) const {
		return static_cast<lightset::PortDir>((m_Params.nDirection >> (nPortIndex * 2)) & 0x3);
	}

	uint16_t portdir_set(const uint32_t nPortIndex, const lightset::PortDir portDir) {
		return static_cast<uint16_t>((static_cast<uint32_t>(portDir) & 0x3) << (nPortIndex * 2));
	}

	lightset::MergeMode mergemode_get(const uint32_t nPortIndex) {
		return static_cast<lightset::MergeMode>((m_Params.nMergeMode >> (nPortIndex * 2)) & 0x3);
	}

	artnet::PortProtocol protocol_get(const uint32_t nPortIndex) {
		return static_cast<artnet::PortProtocol>((m_Params.nProtocol >> (nPortIndex * 2)) & 0x3);
	}

	void Dump();
	void callbackFunction(const char *pLine);
	void SetBool(const uint8_t nValue, const uint32_t nMask);
	bool isMaskSet(uint32_t nMask) const {
		return (m_Params.nSetList & nMask) == nMask;
	}
	bool isOutputStyleSet(uint8_t nMask) const {
		return (m_Params.nOutputStyle & nMask) == nMask;
	}

private:
	artnetparams::Params m_Params;
};

#endif /* ARTNETPARAMS_H_ */
