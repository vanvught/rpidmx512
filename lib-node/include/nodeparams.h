/**
 * @file nodeparams.h
 *
 */
/* Copyright (C) 2022 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef NODEPARAMS_H_
#define NODEPARAMS_H_

#include <cstdint>

#include "node.h"

#if !defined(LIGHTSET_PORTS)
# error LIGHTSET_PORTS is not defined
#endif

namespace nodeparams {
static constexpr uint16_t clear_mask(const uint32_t i) {
	return ~(static_cast<uint16_t>(1U << (i + 8)) | static_cast<uint16_t>(1U << i));
}

static constexpr uint16_t shift_left(const uint32_t nValue, const uint32_t i) {
	return static_cast<uint16_t>((nValue & 0x1) << i);
}

#if LIGHTSET_PORTS > 8
 static constexpr uint32_t MAX_PORTS = 4;
#else
 static constexpr uint32_t MAX_PORTS = LIGHTSET_PORTS;
#endif

struct Params {										///<  LIGHTSET_PORTS = 8
	uint32_t nSetList;								///<  4
	// Node
	uint16_t nUniverse[MAX_PORTS];					///< 16
	uint16_t nDirection;							///<  2
	uint16_t nMergeMode;							///<  2
	// Art-Net
	uint16_t nProtocol;								///<  2
	uint16_t nRdm;									///<  2
	uint8_t aShortName[node::SHORT_NAME_LENGTH];	///< 18
	uint8_t aLongName[node::LONG_NAME_LENGTH];		///< 64
	uint32_t nDestinationIp[MAX_PORTS];				///< 32
	// sACN E1.31
	uint8_t nPriority[MAX_PORTS];					///<  8
	// Node
	uint8_t nPersonality;							///<  1
	uint8_t nFailSafe;								///<  1
} __attribute__((packed));

static_assert(sizeof(struct Params) <= 192, "struct Params is too large");

struct Mask {
	// Node
	static constexpr auto PERSONALITY  			= (1U << 0);
	static constexpr auto UNIVERSE_A    		= (1U << 1);
	static constexpr auto UNIVERSE_B    		= (1U << 2);
	static constexpr auto UNIVERSE_C    		= (1U << 3);
	static constexpr auto UNIVERSE_D    		= (1U << 4);
	static constexpr auto UNIVERSE_E    		= (1U << 5);
	static constexpr auto UNIVERSE_F    		= (1U << 6);
	static constexpr auto UNIVERSE_G    		= (1U << 7);
	static constexpr auto UNIVERSE_H    		= (1U << 8);
	static constexpr auto DISABLE_MERGE_TIMEOUT	= (1U << 9);
	// Art-Net
	static constexpr auto FAILSAFE   			= (1U << 19);
	static constexpr auto ENABLE_RDM    		= (1U << 20);
	static constexpr auto LONG_NAME     		= (1U << 21);
	static constexpr auto SHORT_NAME   			= (1U << 22);
	static constexpr auto MAP_UNIVERSE0 		= (1U << 23);
	// sACN E1.31
	static constexpr auto PRIORITY_A    		= (1U << 24);
	static constexpr auto PRIORITY_B    		= (1U << 25);
	static constexpr auto PRIORITY_C    		= (1U << 26);
	static constexpr auto PRIORITY_D    		= (1U << 27);
	static constexpr auto PRIORITY_E    		= (1U << 28);
	static constexpr auto PRIORITY_F    		= (1U << 29);
	static constexpr auto PRIORITY_G    		= (1U << 30);
	static constexpr auto PRIORITY_H    		= (1U << 31);
};

constexpr auto MASK_NODE = Mask::PERSONALITY |
						   Mask::UNIVERSE_A | Mask::UNIVERSE_B | Mask::UNIVERSE_C | Mask::UNIVERSE_D |
						   Mask::UNIVERSE_E | Mask::UNIVERSE_F | Mask::UNIVERSE_G | Mask::UNIVERSE_H |
						   Mask::DISABLE_MERGE_TIMEOUT;

constexpr auto MASK_ARTNET = Mask::FAILSAFE| Mask::ENABLE_RDM
					 | Mask::LONG_NAME
					 | Mask::SHORT_NAME
					 | Mask::MAP_UNIVERSE0;

constexpr auto MASK_E131 = Mask::PRIORITY_A | Mask::PRIORITY_B | Mask::PRIORITY_C | Mask::PRIORITY_D |
						   Mask::PRIORITY_E | Mask::PRIORITY_F | Mask::PRIORITY_G | Mask::PRIORITY_H;
}  // namespace nodeparams

class NodeParamsStore {
public:
	virtual ~NodeParamsStore() {}

	virtual void Update(const struct nodeparams::Params *pParams)=0;
	virtual void Copy(struct nodeparams::Params *pParams)=0;
};

class NodeParams {
public:
	NodeParams(NodeParamsStore *pNodeParamsStore = nullptr, node::Personality personality = node::Personality::UNKNOWN);

	bool Load();
	void Load(const char *pBuffer, uint32_t nLength);

	void Builder(const struct nodeparams::Params *pParams, char *pBuffer, uint32_t nLength, uint32_t& nSize);
	void Save(char *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Set(uint32_t nPortIndexOffset = 0);

	void Dump();

	node::Personality GetPersonality(bool &IsSet) const {
		IsSet = isMaskSet(nodeparams::Mask::PERSONALITY);

		if (!IsSet) {
			return node::Personality::NODE;
		}

		if (m_Params.nPersonality >= static_cast<uint8_t>(node::Personality::UNKNOWN)) {
			return node::Personality::NODE;
		}

		return static_cast<node::Personality>(m_Params.nPersonality);
	}

    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *pLine);
    bool isMaskSet(const uint32_t nMask) const {
    	return (m_Params.nSetList & nMask) == nMask;
    }
    void ResetSet(node::Personality personality);

private:
    NodeParamsStore *m_pNodeParamsStore;
    node::Personality m_Personality;

    static nodeparams::Params m_Params;
};

#endif /* NODEPARAMS_H_ */
