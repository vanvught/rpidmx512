/**
 * @file artnetrdmcontroller.cpp
 *
 */
/* Copyright (C) 2017-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cstring>
#include <cassert>

#include "hardware.h"

#include "artnetnode.h"
#include "artnetrdmcontroller.h"

#include "rdm.h"
#include "rdm_e120.h"
#include "rdmdevicecontroller.h"
#include "rdmdiscovery.h"

#include "debug.h"

RDMTod ArtNetRdmController::m_pRDMTod[artnetnode::MAX_PORTS];
TRdmMessage ArtNetRdmController::s_rdmMessage;

ArtNetRdmController::ArtNetRdmController(): RDMDiscovery(RDMDeviceController::GetUID()) {
	DEBUG_ENTRY
	s_rdmMessage.start_code = E120_SC_RDM;
	DEBUG_EXIT
}

const uint8_t *ArtNetRdmController::Handler(uint32_t nPortIndex, const uint8_t *pRdmData) {
	assert(nPortIndex < artnetnode::MAX_PORTS);

	if (pRdmData == nullptr) {
		return nullptr;
	}

	Hardware::Get()->WatchdogFeed();

	while (nullptr != Rdm::Receive(nPortIndex)) {
		// Discard late responses
		Hardware::Get()->WatchdogFeed();
	}

	const auto *pRdmMessageNoSc = reinterpret_cast<const TRdmMessageNoSc*>(const_cast<uint8_t*>(pRdmData));
	auto *pRdmCommand = reinterpret_cast<uint8_t*>(&s_rdmMessage);

	memcpy(&pRdmCommand[1], pRdmData, static_cast<size_t>(pRdmMessageNoSc->message_length + 2));

#ifndef NDEBUG
	RDMMessage::Print(pRdmCommand);
#endif

	Rdm::SendRaw(nPortIndex, pRdmCommand, pRdmMessageNoSc->message_length + 2U);

	const auto *pResponse = Rdm::ReceiveTimeOut(nPortIndex, 60000);

#ifndef NDEBUG
	RDMMessage::Print(pResponse);
#endif
	return pResponse;
}

bool ArtNetRdmController::RdmReceive(uint32_t nPortIndex, uint8_t *pRdmData) {
	assert(nPortIndex < artnetnode::MAX_PORTS);
	assert(pRdmData != nullptr);

	auto *pData = Rdm::Receive(nPortIndex);

	if (pData == nullptr) {
		return false;
	}

	auto *pRdmMessage = reinterpret_cast<const struct TRdmMessage *>(pData);

	auto *pUid = pRdmMessage->destination_uid;

	auto bIsRdmPacketBroadcast = (memcmp(pUid, UID_ALL, RDM_UID_SIZE) == 0);
	auto bIsRdmPacketForMe = false;

	if (!bIsRdmPacketBroadcast) {
		bIsRdmPacketForMe = m_pRDMTod[nPortIndex].Exist(pUid);
	}

	if ((!bIsRdmPacketForMe) && (!bIsRdmPacketBroadcast)) {
		return false;
	}

	if ((pRdmMessage->command_class == E120_GET_COMMAND) || (pRdmMessage->command_class == E120_SET_COMMAND) ) {
		memcpy(pRdmData, &pData[1], pRdmMessage->message_length);
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
					auto *pResponse = reinterpret_cast<struct TRdmDiscoveryMsg *>(&s_rdmMessage);

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

				s_rdmMessage.param_data_length = 2;
				s_rdmMessage.param_data[0] = 0x00;	// Control Field
				s_rdmMessage.param_data[1] = 0x00;	// Control Field

				RespondMessageAck(nPortIndex, pUid, pRdmMessage);
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

				s_rdmMessage.param_data_length = 2;
				s_rdmMessage.param_data[0] = 0x00;	// Control Field
				s_rdmMessage.param_data[1] = 0x00;	// Control Field

				RespondMessageAck(nPortIndex, pUid, pRdmMessage);;
				return false;
			}
		}
	}

	return false;
}

void ArtNetRdmController::RespondMessageAck(uint32_t nPortIndex, const uint8_t *pUid, const struct TRdmMessage *pRdmMessage) {
	s_rdmMessage.start_code = E120_SC_RDM;
	s_rdmMessage.sub_start_code = pRdmMessage->sub_start_code;
	s_rdmMessage.transaction_number = pRdmMessage->transaction_number;
	s_rdmMessage.message_count = 0;
	s_rdmMessage.sub_device[0] = pRdmMessage->sub_device[0];
	s_rdmMessage.sub_device[1] = pRdmMessage->sub_device[1];
	s_rdmMessage.command_class = static_cast<uint8_t>(pRdmMessage->command_class + 1);
	s_rdmMessage.param_id[0] = pRdmMessage->param_id[0];
	s_rdmMessage.param_id[1] = pRdmMessage->param_id[1];
	s_rdmMessage.message_length = static_cast<uint8_t>(RDM_MESSAGE_MINIMUM_SIZE + s_rdmMessage.param_data_length);
	s_rdmMessage.slot16.response_type = E120_RESPONSE_TYPE_ACK;

	for (uint32_t i = 0; i < RDM_UID_SIZE; i++) {
		s_rdmMessage.destination_uid[i] = pRdmMessage->source_uid[i];
		s_rdmMessage.source_uid[i] = pUid[i];
	}

	auto *pResponse = reinterpret_cast<uint8_t *>(&s_rdmMessage);

	uint16_t nChecksum = 0;
	uint32_t i;

	for (i = 0; i < s_rdmMessage.message_length; i++) {
		nChecksum = static_cast<uint16_t>(nChecksum + pResponse[i]);
	}

	pResponse[i++] = static_cast<uint8_t>(nChecksum >> 8);
	pResponse[i] = static_cast<uint8_t>(nChecksum & 0XFF);

	const auto nLength = static_cast<uint16_t>(s_rdmMessage.message_length + RDM_MESSAGE_CHECKSUM_SIZE);

	Rdm::SendRawRespondMessage(nPortIndex, pResponse, nLength);
}
