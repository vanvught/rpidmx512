/**
 * @file artnetrdmcontroller.cpp
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if !defined(__clang__)
# pragma GCC push_options
# pragma GCC optimize ("O2")
# pragma GCC optimize ("no-tree-loop-distribute-patterns")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "artnetnode.h"
#include "artnetrdmcontroller.h"

#include "rdm.h"
#include "rdmconst.h"
#include "rdm_e120.h"

#include "debug.h"

RDMTod ArtNetRdmController::m_pRDMTod[artnetnode::MAX_PORTS];

static void respond_message_ack(const uint32_t nPortIndex, struct TRdmMessage *pRdmMessage) {
	assert(pRdmMessage->start_code == E120_SC_RDM);

	pRdmMessage->message_count = 0;
	pRdmMessage->command_class = static_cast<uint8_t>(pRdmMessage->command_class + 1U);
	pRdmMessage->message_length = static_cast<uint8_t>(RDM_MESSAGE_MINIMUM_SIZE + pRdmMessage->param_data_length);
	pRdmMessage->slot16.response_type = E120_RESPONSE_TYPE_ACK;

	for (uint32_t i = 0; i < RDM_UID_SIZE; i++) {
		auto nUid = pRdmMessage->destination_uid[i];
		pRdmMessage->destination_uid[i] = pRdmMessage->source_uid[i];
		pRdmMessage->source_uid[i] = nUid;
	}

	uint16_t nChecksum = 0;
	uint32_t i;

	auto *pRdmData = reinterpret_cast<uint8_t *>(pRdmMessage);

	for (i = 0; i < pRdmMessage->message_length; i++) {
		nChecksum = static_cast<uint16_t>(nChecksum + pRdmData[i]);
	}

	pRdmData[i++] = static_cast<uint8_t>(nChecksum >> 8);
	pRdmData[i++] = static_cast<uint8_t>(nChecksum & 0XFF);

	Rdm::SendRawRespondMessage(nPortIndex, reinterpret_cast<uint8_t *>(pRdmMessage), i);
}

bool ArtNetRdmController::RdmReceive(const uint32_t nPortIndex, const uint8_t *pRdmData) {
	assert(nPortIndex < artnetnode::MAX_PORTS);
	assert(pRdmData != nullptr);

	auto *pRdmMessage = reinterpret_cast<const struct TRdmMessage *>(pRdmData);
	auto *pUid = pRdmMessage->destination_uid;
	auto bIsRdmPacketForMe = false;

	auto bIsRdmPacketBroadcast = (memcmp(pUid, UID_ALL, RDM_UID_SIZE) == 0);

	if (!bIsRdmPacketBroadcast) {
		bIsRdmPacketBroadcast = (memcmp(&pRdmMessage->destination_uid[2], UID_ALL, 4) == 0);

		if (!bIsRdmPacketBroadcast) {
			bIsRdmPacketForMe = m_pRDMTod[nPortIndex].Exist(pUid);
		}
	}

	if ((!bIsRdmPacketForMe) && (!bIsRdmPacketBroadcast)) {
		return false;
	}

	if ((pRdmMessage->command_class == E120_GET_COMMAND) || (pRdmMessage->command_class == E120_SET_COMMAND) ) {
		return true;
	}

	if (bIsRdmPacketBroadcast) {
		pUid = m_pRDMTod[nPortIndex].Next();
	}

	if (pRdmMessage->command_class == E120_DISCOVERY_COMMAND) {
		const auto nParamId = static_cast<uint16_t>((pRdmMessage->param_id[0] << 8) + pRdmMessage->param_id[1]);

		if (nParamId == E120_DISC_UNIQUE_BRANCH) {
			if (!m_pRDMTod[nPortIndex].IsMuted()) {
				if ((memcmp(pRdmMessage->param_data, pUid, RDM_UID_SIZE) <= 0) && (memcmp(pUid, pRdmMessage->param_data + 6, RDM_UID_SIZE) <= 0)) {
					auto *pResponse = reinterpret_cast<struct TRdmDiscoveryMsg *>(const_cast<uint8_t *>(pRdmData));

					uint16_t nChecksum = 6 * 0xFF;

					for (uint32_t i = 0; i < 7; i++) {
						pResponse->header_FE[i] = 0xFE;
					}

					pResponse->header_AA = 0xAA;

					for (uint32_t i = 0; i < 6; i++) {
						pResponse->masked_device_id[i + i] = pUid[i] | 0xAA;
						pResponse->masked_device_id[i + i + 1] = pUid[i] | 0x55;
						nChecksum = static_cast<uint16_t>(nChecksum + pUid[i]);
					}

					pResponse->checksum[0] = static_cast<uint8_t>((nChecksum >> 8) | 0xAA);
					pResponse->checksum[1] = static_cast<uint8_t>((nChecksum >> 8) | 0x55);
					pResponse->checksum[2] = static_cast<uint8_t>((nChecksum & 0xFF) | 0xAA);
					pResponse->checksum[3] = static_cast<uint8_t>((nChecksum & 0xFF) | 0x55);

					Rdm::SendDiscoveryRespondMessage(nPortIndex, reinterpret_cast<uint8_t *>(pResponse), sizeof(struct TRdmDiscoveryMsg));
					return false;
				}
			}
		} else if (nParamId == E120_DISC_UN_MUTE) {
			if (pRdmMessage->param_data_length != 0) {
				/* The response RESPONSE_TYPE_NACK_REASON shall only be used in conjunction
				 * with the Command Classes GET_COMMAND_RESPONSE & SET_COMMAND_RESPONSE.
				 */
				return false;
			}

			if (!bIsRdmPacketBroadcast && bIsRdmPacketForMe) {
				m_pRDMTod[nPortIndex].UnMute();

				auto *pRdmMessage = reinterpret_cast<struct TRdmMessage *>(const_cast<uint8_t *>(pRdmData));

				pRdmMessage->param_data_length = 2;
				pRdmMessage->param_data[0] = 0x00;	// Control Field
				pRdmMessage->param_data[1] = 0x00;	// Control Field

				respond_message_ack(nPortIndex, pRdmMessage);
			} else {
				m_pRDMTod[nPortIndex].UnMuteAll();
			}

			return false;
		} else if (nParamId == E120_DISC_MUTE) {
			if (pRdmMessage->param_data_length != 0) {
				/* The response RESPONSE_TYPE_NACK_REASON shall only be used in conjunction
				 * with the Command Classes GET_COMMAND_RESPONSE & SET_COMMAND_RESPONSE.
				 */
				return false;
			}

			if (bIsRdmPacketForMe) {
				m_pRDMTod[nPortIndex].Mute();

				auto *pRdmMessage = reinterpret_cast<struct TRdmMessage *>(const_cast<uint8_t *>(pRdmData));

				pRdmMessage->param_data_length = 2;
				pRdmMessage->param_data[0] = 0x00;	// Control Field
				pRdmMessage->param_data[1] = 0x00;	// Control Field

				respond_message_ack(nPortIndex, pRdmMessage);;
				return false;
			}
		}
	}

	return false;
}
