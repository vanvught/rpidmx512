/**
 * @file node.h
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

#ifndef NODE_H_
#define NODE_H_

#include <cstdint>
#include <cstring>

#include "nodestore.h"

// DMX Output
#include "dmxconfigudp.h"
// Art-Net
#include "artnetnode.h"
#include "artnet4handler.h"
#include "artnet.h"
#include "artnetrdmcontroller.h"
#include "dmx/artnetdmxinput.h"
// sACN E1.31
#include "e131bridge.h"
#include "e131.h"
#include "dmx/e131dmxinput.h"

namespace node {
enum class Personality {
	NODE, ARTNET, E131, UNKNOWN
};

namespace defaults {
static constexpr auto PERSONALITY = Personality::ARTNET;
static constexpr auto FAILSAFE = lightset::FailSafe::HOLD;
}

namespace universe {
static constexpr auto MAX = e131::universe::MAX;
}  // namespace universe

namespace priority {
static constexpr auto LOWEST = e131::priority::LOWEST;
static constexpr auto DEFAULT = e131::priority::DEFAULT;
static constexpr auto HIGHEST = e131::priority::HIGHEST;
}  // namespace priority

static constexpr auto SHORT_NAME_LENGTH = artnet::SHORT_NAME_LENGTH;
static constexpr auto LONG_NAME_LENGTH = artnet::LONG_NAME_LENGTH;

enum class PortProtocol {
	ARTNET, SACN, UNKNOWN
};

inline static Personality get_personality(const char *p) {
	if (memcmp(p, "sacn", 4) == 0) {
		return Personality::E131;
	}

	if (memcmp(p, "artnet", 6) == 0) {
		return Personality::ARTNET;
	}

	return defaults::PERSONALITY;
}

inline static const char* get_personality(const Personality personality) {
	switch (personality) {
	case Personality::NODE:
		return "node";
		break;
	case Personality::ARTNET:
		return "artnet";
		break;
	case Personality::E131:
		return "sacn";
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	__builtin_unreachable();
	return "";
}

inline static const char* get_personality_full(const Personality personality) {
	switch (personality) {
	case Personality::NODE:
		return "Node";
		break;
	case Personality::ARTNET:
		return "Art-Net 4";
		break;
	case Personality::E131:
		return "sACN E1.31";
		break;
	case Personality::UNKNOWN:
		return "Unknown";
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}

	__builtin_unreachable();
	return "";
}

struct Option {
	static constexpr uint32_t ENABLE_RDM = (1U << 0);
};
}  // namespace node

///< The personality classes must be private
///< All configuration must be done with public methods from Node
///< The personality is set at start/reboot only -> needed for DMX512 input framework

class Node: ArtNetNode, ArtNet4Handler, E131Bridge {
public:
	Node(node::Personality personality, NodeStore *pNodeStore);
	~Node() override;

	/**
	 * Node
	 */

	node::Personality GetPersonality() const {
		return m_Personality;
	}

	void SetFailSafe(const lightset::FailSafe failsafe);
	lightset::FailSafe GetFailSafe();

	void SetLightSet(LightSet *pLightSet);

	uint32_t GetPorts() const {
		return m_nPorts;
	}

	void SetUniverse(uint32_t nPortIndex, lightset::PortDir portDir, uint16_t nUniverse);
	bool GetUniverse(uint32_t nPortIndex, uint16_t& nUniverse, lightset::PortDir portDir);

	void SetMergeMode(uint32_t nPortIndex, lightset::MergeMode mergeMode);
	lightset::MergeMode GetMergeMode(uint32_t nPortIndex);

	uint32_t GetActiveInputPorts() const {
		switch (m_Personality) {
			case node::Personality::ARTNET:
				return ArtNetNode::GetActiveInputPorts();
				break;
			case node::Personality::E131:
				return E131Bridge::GetActiveInputPorts();
				break;
			default:
				break;
		}

		return 0;
	}

	uint32_t GetActiveOutputPorts() const {
		switch (m_Personality) {
			case node::Personality::ARTNET:
				return ArtNetNode::GetActiveOutputPorts();
				break;
			case node::Personality::E131:
				return E131Bridge::GetActiveOutputPorts();
				break;
			default:
				break;
		}

		return 0;
	}

	uint32_t GetActivePorts() const {
		return GetActiveInputPorts() + GetActiveOutputPorts();
	}

	uint32_t RdmGetUidCount(uint32_t nPortIndex) {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		if (m_pArtNetRdmController == nullptr) {
			return 0;
		}
		return m_pArtNetRdmController->GetUidCount(nPortIndex);
	}

	bool RdmCopyUidEntry(uint32_t nPortIndex, uint32_t nIndex, uint8_t uid[RDM_UID_SIZE]) {
		assert(nPortIndex < artnetnode::MAX_PORTS);
		if (m_pArtNetRdmController == nullptr) {
			return false;
		}
		return m_pArtNetRdmController->CopyTodEntry(nPortIndex, nIndex, uid);
	}

	/**
	 * Extra's
	 */

	void SetDisableMergeTimeout(bool bDisable);
	bool GetDisableMergeTimeout();

	/**
	 * Art-Net 4
	 */

	void SetShortName(const char *pShortName) {
		ArtNetNode::SetShortName(pShortName);
	}
	const char *GetShortName() const {
		return ArtNetNode::GetShortName();
	}

	void SetLongName(const char *pLongName) {
		ArtNetNode::SetLongName(pLongName);
	}
	const char *GetLongName() const {
		return ArtNetNode::GetLongName();
	}

	void SetDestinationIp(uint32_t nPortIndex, uint32_t nDestinationIp) {
		ArtNetNode::SetDestinationIp(nPortIndex, nDestinationIp);
	}
	uint32_t GetDestinationIp(uint32_t nPortIndex) const {
		return ArtNetNode::GetDestinationIp(nPortIndex);
	}

	void SetProtocol(uint32_t nPortIndex, node::PortProtocol portProtocol) {
		const auto ArtNetPortProtocol = (portProtocol == node::PortProtocol::SACN ? artnet::PortProtocol::SACN : artnet::PortProtocol::ARTNET);
		ArtNetNode::SetPortProtocol(nPortIndex, ArtNetPortProtocol);
	}
	node::PortProtocol GetProtocol(uint32_t nPortIndex) const {
		return ArtNetNode::GetPortProtocol(nPortIndex) == artnet::PortProtocol::SACN ? node::PortProtocol::SACN : node::PortProtocol::ARTNET;
	}

	void SetMapUniverse0(bool bMapUniverse0) {
		ArtNetNode::SetMapUniverse0(bMapUniverse0);
	}
	bool IsMapUniverse0() {
		return ArtNetNode::IsMapUniverse0();
	}

	void SetRdmUID(const uint8_t *pUid, bool bSupportsLLRP) {
		ArtNetNode::SetRdmUID(pUid, bSupportsLLRP);
	}

	void SetRdm(const bool bSet) {
		if (bSet) {
			OptionEnable(node::Option::ENABLE_RDM);
		} else {
			OptionDisable(node::Option::ENABLE_RDM);
		}

		EventSetRdm(true);
	}
	bool IsRdm() const {
		return IsOptionSet(node::Option::ENABLE_RDM);
	}

	void SetRmd(uint32_t nPortIndex, bool bEnable) {
		ArtNetNode::SetRmd(nPortIndex, bEnable);
	}
	bool GetRdm(uint32_t nPortIndex) const {
		return ArtNetNode::GetRdm(nPortIndex);
	}

	void SetArtNetTrigger(ArtNetTrigger *pArtNetTrigger) {
		ArtNetNode::SetArtNetTrigger(pArtNetTrigger);
	}

	/**
	 * sACN E1.31
	 */

	void SetPriority(uint32_t nPortIndex, uint8_t nPriority) {
		E131Bridge::SetPriority(nPriority, nPortIndex);
	}
	uint8_t GetPriority(uint32_t nPortIndex) const {
		return E131Bridge::GetPriority(nPortIndex);
	}

	void Start();
	void Stop();

	void Print();

	void Run() {
		if (m_Personality == node::Personality::ARTNET ) {
			ArtNetNode::Run();
		}

		E131Bridge::Run();

		if (m_pDmxConfigUdp != nullptr) {
			m_pDmxConfigUdp->Run();
		}
	}

	static Node* Get() {
		return s_pThis;
	}

private:
	void EventSetUniverse(bool bOverwriteIsStarted = false);

	void OptionEnable(const uint32_t nOption) {
		m_nOptions |= nOption;
	}

	void OptionDisable(const uint32_t nOption) {
		m_nOptions &= ~nOption;
	}

    bool IsOptionSet(const uint32_t nOption) const {
    	return (m_nOptions & nOption) == nOption;
    }

    // Art-Net

	void EventSetRdm(bool bOverwriteIsStarted = false);

	// Art-Net 4 Handler

	void SetPort(uint32_t nPortIndex, lightset::PortDir dir) override;

	void HandleAddress(uint8_t nCommand) override;
	uint8_t GetStatus(uint32_t nPortIndex) override;

	bool IsStatusChanged() override {
		return E131Bridge::IsStatusChanged();
	}

	void SetLedBlinkMode(ledblink::Mode mode) override {
		E131Bridge::SetEnableDataIndicator(mode == ledblink::Mode::NORMAL);
		LedBlink::Get()->SetMode(mode);
	}

private:
	node::Personality m_Personality { node::Personality::ARTNET };
	uint32_t m_nOptions { 0 };
	bool m_IsStarted { false };
	uint32_t m_nPorts;
	// DMX Output
	DmxConfigUdp *m_pDmxConfigUdp { nullptr };
	// Art-Net
	ArtNetRdmController *m_pArtNetRdmController { nullptr };
	ArtNetDmxInput m_ArtNetDmxInput;
	// sACN E1.31
	E131DmxInput m_E131DmxInput;

	static Node *s_pThis;
};

#endif /* NODE_H_ */
