/*
 * setrdm.cpp
 */

#include <cstdint>

#include "artnetnode.h"

#include "debug.h"

void ArtNetNode::SetRdm(const uint32_t nPortIndex, const bool bEnable) {
	DEBUG_ENTRY
	assert(nPortIndex < artnetnode::MAX_PORTS);

	const auto isEnabled = !((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) == artnet::GoodOutputB::RDM_DISABLED);

	if (isEnabled == bEnable) {
		DEBUG_EXIT
		return;
	}

	if (!bEnable) {
		m_OutputPort[nPortIndex].GoodOutputB |= artnet::GoodOutputB::RDM_DISABLED;
	} else {
		m_OutputPort[nPortIndex].GoodOutputB &= static_cast<uint8_t>(~artnet::GoodOutputB::RDM_DISABLED);
	}

	if (m_State.status == artnetnode::Status::ON) {
		if (m_pArtNetStore != nullptr) {
			m_pArtNetStore->SaveRdmEnabled(nPortIndex, bEnable);
		}

		artnet::display_rdm_enabled(nPortIndex, bEnable);
	}


	DEBUG_EXIT
}
