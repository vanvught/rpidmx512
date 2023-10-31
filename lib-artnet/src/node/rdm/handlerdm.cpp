/*
 * handlerdm.cpp
 *
 *  Created on: 24 okt. 2023
 *      Author: arjanvanvught
 */

#include <cstdint>

#include "artnetnode.h"

#include "debug.h"

/**
 * An Output Gateway must not interpret receipt of an ArtTodRequest
 * as an instruction to perform full RDM Discovery on the DMX512 physical layer;
 * it is just a request to send the ToD back to the controller.
 */
void ArtNetNode::HandleTodRequest() {
	DEBUG_ENTRY

	const auto *const pArtTodRequest = reinterpret_cast<artnet::ArtTodRequest *>(m_pReceiveBuffer);
	const auto nAddCount = pArtTodRequest->AddCount & 0x1f;

	for (auto nCount = 0; nCount < nAddCount; nCount++) {
		const auto portAddress = static_cast<uint16_t>((pArtTodRequest->Net << 8)) | static_cast<uint16_t>((pArtTodRequest->Address[nCount]));

		for (uint32_t nPortIndex = 0; nPortIndex < artnetnode::MAX_PORTS; nPortIndex++) {
			if ((m_OutputPort[nPortIndex].GoodOutputB & artnet::GoodOutputB::RDM_DISABLED) == artnet::GoodOutputB::RDM_DISABLED) {
				continue;
			}

			if ((portAddress == m_Node.Port[nPortIndex].PortAddress) && (m_Node.Port[nPortIndex].direction == lightset::PortDir::OUTPUT)) {
				SendTod(nPortIndex);
			}
		}
	}

	DEBUG_EXIT
}
