/**
 * @file rdmddiscovery.cpp
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@gd32-dmx.org
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
#include <cstdio>
#include <cassert>

#include "rdmdiscovery.h"
#include "hardware.h"

#include "debug.h"

namespace rdmdiscovery {
#ifndef NDEBUG
const static char *StateName[] = {
		"IDLE",
		"UNMUTE",
		"MUTE",
		"DISCOVERY",
		"DISCOVERY_SINGLE_DEVICE",
		"DUB",
		"QUICKFIND",
		"QUICKFIND_DISCOVERY",
		"LATE_RESPONSE",
		"FINISHED"
};
#endif

typedef union cast {
	uint64_t uint;
	uint8_t uid[RDM_UID_SIZE];
} _cast;

static _cast uuid_cast;

static const uint8_t *convert_uid(uint64_t nUid) {
	uuid_cast.uint = __builtin_bswap64(nUid << 16);
	return uuid_cast.uid;
}

#ifndef NDEBUG
static void print_uid([[maybe_unused]] const uint8_t *pUid) {
	printf("%.2x%.2x:%.2x%.2x%.2x%.2x", pUid[0], pUid[1], pUid[2], pUid[3], pUid[4], pUid[5]);
}

static void print_uid([[maybe_unused]] uint64_t nUid) {
	print_uid(convert_uid(nUid));
}
#endif

}  // namespace rdmdiscovery

#define NEW_STATE(state, late)	NewState (state, late, __LINE__);
#define SAVED_STATE()			SavedState (__LINE__);

RDMDiscovery::RDMDiscovery(const uint8_t *pUid) {
	memcpy(m_Uid, pUid, RDM_UID_SIZE);
	m_Message.SetSrcUid(pUid);

#ifndef NDEBUG
	printf("Uid : ");
	rdmdiscovery::print_uid(m_Uid);
	puts("");
#endif
}

uint32_t RDMDiscovery::CopyWorkingQueue(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nSize = static_cast<int32_t>(nOutBufferSize);
	int32_t nIndex = 0;
	int32_t nLength = 0;
	uint8_t pLowerBound[RDM_UID_SIZE];
	uint8_t pUpperBound[RDM_UID_SIZE];

	while (nIndex <= m_Discovery.stack.nTop) {
		memcpy(pLowerBound, rdmdiscovery::convert_uid(m_Discovery.stack.items[nIndex].nLowerBound), RDM_UID_SIZE);
		memcpy(pUpperBound, rdmdiscovery::convert_uid(m_Discovery.stack.items[nIndex].nUpperBound), RDM_UID_SIZE);

		nLength += snprintf(&pOutBuffer[nLength], static_cast<size_t>(nSize - nLength),
				"\"%.2x%.2x:%.2x%.2x%.2x%.2x-%.2x%.2x:%.2x%.2x%.2x%.2x\",",
				pLowerBound[0], pLowerBound[1], pLowerBound[2], pLowerBound[3], pLowerBound[4], pLowerBound[5],
				pUpperBound[0], pUpperBound[1], pUpperBound[2], pUpperBound[3], pUpperBound[4], pUpperBound[5]);

		nIndex++;
	}

	if (nLength == 0) {
		return 0;
	}

	pOutBuffer[nLength - 1] = '\0';

	return static_cast<uint32_t>(nLength - 1);
}

bool RDMDiscovery::Full(const uint32_t nPortIndex, RDMTod *pRDMTod) {
	DEBUG_ENTRY
	pRDMTod->Reset();
	const auto b = Start(nPortIndex, pRDMTod, false);
	DEBUG_EXIT
	return b;
}

bool RDMDiscovery::Incremental(const uint32_t nPortIndex, RDMTod *pRDMTod) {
	DEBUG_ENTRY
	m_Mute.nTodEntries = pRDMTod->GetUidCount();
	const auto b = Start(nPortIndex, pRDMTod, true);
	DEBUG_EXIT
	return b;
}

bool RDMDiscovery::Start(const uint32_t nPortIndex, RDMTod *pRDMTod, const bool doIncremental) {
	DEBUG_ENTRY

	if (m_State != rdmdiscovery::State::IDLE) {
		DEBUG_PUTS("Is already running.");
		DEBUG_EXIT
		return false;
	}

	m_nPortIndex = nPortIndex;
	m_pRDMTod = pRDMTod;

	m_doIncremental = doIncremental;
	m_bIsFinished = false;

	m_UnMute.nCounter = rdmdiscovery::UNMUTE_COUNTER;
	m_UnMute.bCommandRunning = false;

	m_Mute.nCounter = rdmdiscovery::MUTE_COUNTER;
	m_Mute.bCommandRunning = false;

	m_Discovery.stack.nTop = -1;
	m_Discovery.stack.push(0x000000000000, 0xfffffffffffe);
	m_Discovery.nCounter = rdmdiscovery::DISCOVERY_COUNTER;

	m_Discovery.bCommandRunning = false;

	m_DiscoverySingleDevice.nCounter = rdmdiscovery::MUTE_COUNTER;
	m_DiscoverySingleDevice.bCommandRunning = false;

	m_QuikFind.nCounter = rdmdiscovery::QUIKFIND_COUNTER;
	m_QuikFind.bCommandRunning = false;

	m_QuikFindDiscovery.nCounter = rdmdiscovery::QUIKFIND_DISCOVERY_COUNTER;
	m_QuikFindDiscovery.bCommandRunning = false;

	NEW_STATE(rdmdiscovery::State::UNMUTE, false);

#ifndef NDEBUG
	debug.nTreeIndex = 0;
#endif
	DEBUG_EXIT
	return true;
}

bool RDMDiscovery::Stop() {
	DEBUG_ENTRY

	if (m_State == rdmdiscovery::State::IDLE) {
		DEBUG_PUTS("Not running.");
		DEBUG_EXIT
		return false;
	}

	m_bIsFinished = false;

	NEW_STATE(rdmdiscovery::State::IDLE, true);

	DEBUG_EXIT
	return true;
}

bool RDMDiscovery::IsValidDiscoveryResponse(uint8_t *pUid) {
	uint8_t checksum[2];
	uint16_t nRdmChecksum = 6 * 0xFF;
	auto bIsValid = false;

	if (m_pResponse[0] == 0xFE) {
		pUid[0] = m_pResponse[8] & m_pResponse[9];
		pUid[1] = m_pResponse[10] & m_pResponse[11];

		pUid[2] = m_pResponse[12] & m_pResponse[13];
		pUid[3] = m_pResponse[14] & m_pResponse[15];
		pUid[4] = m_pResponse[16] & m_pResponse[17];
		pUid[5] = m_pResponse[18] & m_pResponse[19];

		checksum[0] = m_pResponse[22] & m_pResponse[23];
		checksum[1] = m_pResponse[20] & m_pResponse[21];

		for (uint32_t i = 0; i < 6; i++) {
			nRdmChecksum = static_cast<uint16_t>(nRdmChecksum + pUid[i]);
		}

		if (((nRdmChecksum >> 8) == checksum[1]) && ((nRdmChecksum & 0xFF) == checksum[0])) {
			bIsValid = true;
		}

#ifndef NDEBUG
		rdmdiscovery::print_uid(pUid);
		printf(", checksum %.2x%.2x -> %.4x {%c}\n", checksum[1], checksum[0], nRdmChecksum, bIsValid ? 'Y' : 'N');
#endif
	}

	return bIsValid;
}

void RDMDiscovery::SavedState([[maybe_unused]] const uint32_t nLine) {
	assert(m_SavedState != m_State);
#ifndef NDEBUG
	printf("State %s->%s at line %u\n", rdmdiscovery::StateName[static_cast<uint32_t>(m_State)], rdmdiscovery::StateName[static_cast<uint32_t>(m_SavedState)], nLine);
#endif
	m_State = m_SavedState;
}

void RDMDiscovery::NewState(const rdmdiscovery::State state, const bool doStateLateResponse, [[maybe_unused]] const uint32_t nLine) {
	assert(m_State != state);

	if (doStateLateResponse && (m_State != rdmdiscovery::State::LATE_RESPONSE)) {
#ifndef NDEBUG
		assert(static_cast<uint32_t>(state) < sizeof(rdmdiscovery::StateName) / sizeof(rdmdiscovery::StateName[0]));
		printf("State %s->%s [%s] at line %u\n", rdmdiscovery::StateName[static_cast<uint32_t>(m_State)], rdmdiscovery::StateName[static_cast<uint32_t>(rdmdiscovery::State::LATE_RESPONSE)], rdmdiscovery::StateName[static_cast<uint32_t>(state)],  nLine);
#endif
		m_LateResponse.nMicros = Hardware::Get()->Micros();
		m_SavedState = state;
		m_State = rdmdiscovery::State::LATE_RESPONSE;
	} else {
#ifndef NDEBUG
		printf("State %s->%s at line %u\n", rdmdiscovery::StateName[static_cast<uint32_t>(m_State)], rdmdiscovery::StateName[static_cast<uint32_t>(state)],  nLine);
#endif
		m_State = state;
	}
}

void RDMDiscovery::Process() {
	switch (m_State) {
	case rdmdiscovery::State::LATE_RESPONSE:  ///< LATE_RESPONSE
		m_Message.Receive(m_nPortIndex);

		if ((Hardware::Get()->Micros() - m_LateResponse.nMicros) > rdmdiscovery::LATE_RESPONSE_TIME_OUT) {
			SAVED_STATE();
		}

		return;
		break;
	case rdmdiscovery::State::UNMUTE:  ///< UNMUTE
		if (m_UnMute.nCounter == 0) {
			m_UnMute.nCounter = rdmdiscovery::UNMUTE_COUNTER;
			m_UnMute.bCommandRunning = false;

			if (m_doIncremental) {
				NEW_STATE(rdmdiscovery::State::MUTE, false);
				return;
			}

			NEW_STATE(rdmdiscovery::State::DISCOVERY, false);
			return;
		}

		if (!m_UnMute.bCommandRunning) {
			m_Message.SetPortID(static_cast<uint8_t>(1 + m_nPortIndex));
			m_Message.SetDstUid(UID_ALL);
			m_Message.SetCc(E120_DISCOVERY_COMMAND);
			m_Message.SetPid(E120_DISC_UN_MUTE);
			m_Message.SetPd(nullptr, 0);
			m_Message.Send(m_nPortIndex);

			m_UnMute.nMicros = Hardware::Get()->Micros();
			m_UnMute.bCommandRunning = true;
			return;
		}

		m_Message.Receive(m_nPortIndex);

		if ((Hardware::Get()->Micros() - m_UnMute.nMicros) > rdmdiscovery::RECEIVE_TIME_OUT) {
			assert(m_UnMute.nCounter > 0);
			m_UnMute.nCounter--;
			m_UnMute.bCommandRunning = false;
		}

		return;
		break;
	case rdmdiscovery::State::MUTE:  ///< MUTE
		if (m_Mute.nTodEntries == 0) {
			m_Mute.bCommandRunning = false;
			NEW_STATE(rdmdiscovery::State::DISCOVERY, false);
			return;
		}

		if (m_Mute.nCounter == 0) {
			m_Mute.nCounter = rdmdiscovery::MUTE_COUNTER;
			m_Mute.bCommandRunning = false;
#ifndef NDEBUG
			printf("Device is gone ");rdmdiscovery::print_uid(m_Mute.uid); puts("");
#endif
			m_pRDMTod->Delete(m_Mute.uid);

			if (m_Mute.nTodEntries > 0) {
				m_Mute.nTodEntries--;
			}

			return;
		}

		if (!m_Mute.bCommandRunning) {
			assert(m_Mute.nTodEntries > 0);
			m_pRDMTod->CopyUidEntry(m_Mute.nTodEntries - 1, m_Mute.uid);

			m_Message.SetPortID(static_cast<uint8_t>(1 + m_nPortIndex));
			m_Message.SetDstUid(m_Mute.uid);
			m_Message.SetCc(E120_DISCOVERY_COMMAND);
			m_Message.SetPid(E120_DISC_MUTE);
			m_Message.SetPd(nullptr, 0);
			m_Message.Send(m_nPortIndex);

			m_Mute.nMicros = Hardware::Get()->Micros();
			m_Mute.bCommandRunning = true;
			return;
		}

		m_pResponse = const_cast<uint8_t *>(m_Message.Receive(m_nPortIndex));

		if (m_pResponse != nullptr) {
			assert(m_Mute.nTodEntries > 0);
			m_Mute.nTodEntries--;
			m_Mute.bCommandRunning = false;
			return;
		}

		if ((Hardware::Get()->Micros() - m_Mute.nMicros) > rdmdiscovery::RECEIVE_TIME_OUT) {
			assert(m_Mute.nCounter > 0);
			m_Mute.nCounter--;
			m_Message.Send(m_nPortIndex);
			m_Mute.nMicros = Hardware::Get()->Micros();
		}

		return;
		break;
	case rdmdiscovery::State::DISCOVERY:		///< DISCOVERY
		if (m_Discovery.bCommandRunning) {
			m_pResponse = const_cast<uint8_t *>(m_Message.Receive(m_nPortIndex));

			if ((m_pResponse != nullptr) || (m_Discovery.nCounter == 0)) {
				m_Discovery.bCommandRunning = false;
				NEW_STATE(rdmdiscovery::State::DUB, false);
				return;
			}

			if ((Hardware::Get()->Micros() - m_Discovery.nMicros) > rdmdiscovery::RECEIVE_TIME_OUT) {
				assert(m_Discovery.nCounter > 0);
				m_Discovery.nCounter--;
				m_Message.Send(m_nPortIndex);
				m_Discovery.nMicros = Hardware::Get()->Micros();
			}

			return;
		}

		if (!m_Discovery.stack.pop(m_Discovery.nLowerBound, m_Discovery.nUpperBound)) {
			m_Discovery.bCommandRunning = false;
			NEW_STATE(rdmdiscovery::State::FINISHED, true);
			return;
		}

#ifndef NDEBUG
		debug.tree[debug.nTreeIndex].nLowerBound = m_Discovery.nLowerBound;
		debug.tree[debug.nTreeIndex++].nUpperBound = m_Discovery.nUpperBound;
#endif

		if (m_Discovery.nLowerBound == m_Discovery.nUpperBound) {
			m_QuikFindDiscovery.bCommandRunning = false;
			NEW_STATE(rdmdiscovery::State::DISCOVERY_SINGLE_DEVICE, true);
			return;
		}

		memcpy(m_Discovery.pdl[0], rdmdiscovery::convert_uid(m_Discovery.nLowerBound), RDM_UID_SIZE);
		memcpy(m_Discovery.pdl[1], rdmdiscovery::convert_uid(m_Discovery.nUpperBound), RDM_UID_SIZE);

#ifndef NDEBUG
		printf("DISC_UNIQUE_BRANCH -> "); rdmdiscovery::print_uid(m_Discovery.pdl[0]); printf(" "); rdmdiscovery::print_uid(m_Discovery.pdl[1]); puts("");
#endif

		m_Message.SetDstUid(UID_ALL);
		m_Message.SetCc(E120_DISCOVERY_COMMAND);
		m_Message.SetPid(E120_DISC_UNIQUE_BRANCH);
		m_Message.SetPd(reinterpret_cast<const uint8_t*>(m_Discovery.pdl), 2 * RDM_UID_SIZE);
		m_Message.Send(m_nPortIndex);

		m_Discovery.nCounter = rdmdiscovery::DISCOVERY_COUNTER;
		m_Discovery.nMicros = Hardware::Get()->Micros();
		m_Discovery.bCommandRunning = true;

		return;
		break;
	case rdmdiscovery::State::DISCOVERY_SINGLE_DEVICE:		///< DISCOVERY_SINGLE_DEVICE
		if (m_DiscoverySingleDevice.nCounter == 0) {
			m_DiscoverySingleDevice.nCounter = rdmdiscovery::QUIKFIND_DISCOVERY_COUNTER;
			m_DiscoverySingleDevice.bCommandRunning = false;
			NEW_STATE(rdmdiscovery::State::DISCOVERY, true);
			return;
		}

		if (!m_DiscoverySingleDevice.bCommandRunning) {
			memcpy(m_Discovery.uid, rdmdiscovery::convert_uid(m_Discovery.nLowerBound), RDM_UID_SIZE);

			m_Message.SetCc(E120_DISCOVERY_COMMAND);
			m_Message.SetPid(E120_DISC_MUTE);
			m_Message.SetDstUid(m_Discovery.uid);
			m_Message.SetPd(nullptr, 0);
			m_Message.Send(m_nPortIndex);

			m_DiscoverySingleDevice.nMicros = Hardware::Get()->Micros();
			m_DiscoverySingleDevice.bCommandRunning = true;
			return;
		}

		m_pResponse = const_cast<uint8_t *>(m_Message.Receive(m_nPortIndex));

		if (m_pResponse != nullptr) {
			const auto pResponse = reinterpret_cast<struct TRdmMessage*>(m_pResponse);

			if ((pResponse->command_class == E120_DISCOVERY_COMMAND_RESPONSE) && (memcmp(m_Discovery.uid, pResponse->source_uid, RDM_UID_SIZE) == 0)) {
				m_pRDMTod->AddUid(m_Discovery.uid);
#ifndef NDEBUG
				printf("AddUid : ");
				rdmdiscovery::print_uid(m_Discovery.uid);
				puts("");
#endif

				m_DiscoverySingleDevice.nCounter = rdmdiscovery::QUIKFIND_DISCOVERY_COUNTER;
				m_DiscoverySingleDevice.bCommandRunning = false;
				NEW_STATE(rdmdiscovery::State::DISCOVERY, false);
			}

			return;
		}

		if ((Hardware::Get()->Micros() - m_DiscoverySingleDevice.nMicros) > rdmdiscovery::RECEIVE_TIME_OUT) {
			assert(m_Mute.nCounter > 0);
			m_DiscoverySingleDevice.nCounter--;
			m_Message.Send(m_nPortIndex);
			m_DiscoverySingleDevice.nMicros = Hardware::Get()->Micros();
		}

		return;
		break;
	case rdmdiscovery::State::DUB:	///< DUB
		if (m_pResponse == nullptr) {
#ifndef NDEBUG
			puts("No responses");
#endif
			NEW_STATE(rdmdiscovery::State::DISCOVERY, false);
			return;
		}

		if (IsValidDiscoveryResponse(m_QuikFind.uid)) {
			NEW_STATE(rdmdiscovery::State::QUICKFIND, true);
			return;
		}

		m_Discovery.nMidPosition = ((m_Discovery.nLowerBound & (0x0000800000000000 - 1)) + (m_Discovery.nUpperBound & (0x0000800000000000 - 1))) / 2
				+ (m_Discovery.nUpperBound & (0x0000800000000000) ? 0x0000400000000000 : 0 )
				+ (m_Discovery.nLowerBound & (0x0000800000000000) ? 0x0000400000000000 : 0 );


		m_Discovery.stack.push(m_Discovery.nLowerBound, m_Discovery.nMidPosition);
		m_Discovery.stack.push(m_Discovery.nMidPosition + 1, m_Discovery.nUpperBound);

		NEW_STATE(rdmdiscovery::State::DISCOVERY, true);
		break;
	case rdmdiscovery::State::QUICKFIND:	///< QUICKFIND
		if (m_QuikFind.nCounter == 0) {
			m_QuikFind.bCommandRunning = false;
			NEW_STATE(rdmdiscovery::State::QUICKFIND_DISCOVERY, false);
			return;
		}

		if (!m_QuikFind.bCommandRunning) {
#ifndef NDEBUG
			printf("QuickFind : ");
			rdmdiscovery::print_uid(m_QuikFind.uid);
			puts("");
#endif

			m_Message.SetCc(E120_DISCOVERY_COMMAND);
			m_Message.SetPid(E120_DISC_MUTE);
			m_Message.SetDstUid(m_QuikFind.uid);
			m_Message.SetPd(nullptr, 0);
			m_Message.Send(m_nPortIndex);

			m_QuikFind.nCounter = rdmdiscovery::QUIKFIND_COUNTER;
			m_QuikFind.nMicros = Hardware::Get()->Micros();
			m_QuikFind.bCommandRunning = true;
			return;
		}

		m_pResponse = const_cast<uint8_t *>(m_Message.Receive(m_nPortIndex));

		if ((m_pResponse != nullptr)) {
			const auto pResponse = reinterpret_cast<struct TRdmMessage*>(m_pResponse);

			if ((pResponse->command_class != E120_DISCOVERY_COMMAND_RESPONSE) || ((static_cast<uint16_t>((pResponse->param_id[0] << 8) + pResponse->param_id[1])) != E120_DISC_MUTE)) {
				puts("QUICKFIND invalid response");
				//assert(0);
				return;
			}

			if ((pResponse->command_class == E120_DISCOVERY_COMMAND_RESPONSE) && (memcmp(m_QuikFind.uid, pResponse->source_uid, RDM_UID_SIZE) == 0)) {
				m_pRDMTod->AddUid(m_QuikFind.uid);
#ifndef NDEBUG
				printf("AddUid : ");
				rdmdiscovery::print_uid(m_QuikFind.uid);
				puts("");
#endif
			}

			m_QuikFind.nCounter = rdmdiscovery::QUIKFIND_COUNTER;
			m_QuikFind.bCommandRunning = false;
			NEW_STATE(rdmdiscovery::State::QUICKFIND_DISCOVERY, false);
			return;
		}

		if ((Hardware::Get()->Micros() - m_QuikFind.nMicros) > rdmdiscovery::RECEIVE_TIME_OUT) {
			assert(m_QuikFind.nCounter > 0);
			m_QuikFind.nCounter--;
			m_QuikFind.bCommandRunning = false;
		}

		return;
		break;
	case rdmdiscovery::State::QUICKFIND_DISCOVERY:	///< QUICKFIND_DISCOVERY
		if (m_QuikFindDiscovery.nCounter == 0) {
			m_QuikFindDiscovery.nCounter = rdmdiscovery::QUIKFIND_DISCOVERY_COUNTER;
			m_QuikFindDiscovery.bCommandRunning = false;
			NEW_STATE(rdmdiscovery::State::DISCOVERY, true);
			return;
		}

		if (!m_QuikFindDiscovery.bCommandRunning) {
			m_Message.SetDstUid(UID_ALL);
			m_Message.SetCc(E120_DISCOVERY_COMMAND);
			m_Message.SetPid(E120_DISC_UNIQUE_BRANCH);
			m_Message.SetPd(reinterpret_cast<const uint8_t*>(m_Discovery.pdl), 2 * RDM_UID_SIZE);
			m_Message.Send(m_nPortIndex);

			m_QuikFindDiscovery.nMicros = Hardware::Get()->Micros();
			m_QuikFindDiscovery.bCommandRunning = true;
			return;
		}

		m_pResponse = const_cast<uint8_t *>(m_Message.Receive(m_nPortIndex));

		if ((m_pResponse != nullptr) && (IsValidDiscoveryResponse(m_QuikFind.uid))) {
			m_QuikFindDiscovery.nCounter = rdmdiscovery::QUIKFIND_DISCOVERY_COUNTER;
			m_QuikFindDiscovery.bCommandRunning = false;
			NEW_STATE(rdmdiscovery::State::QUICKFIND, true);
			return;
		}

		if ((m_pResponse != nullptr) && (!IsValidDiscoveryResponse(m_QuikFind.uid))) {
			m_QuikFindDiscovery.nCounter = rdmdiscovery::QUIKFIND_DISCOVERY_COUNTER;
			m_QuikFindDiscovery.bCommandRunning = false;
			NEW_STATE(rdmdiscovery::State::DUB, false);
			return;
		}

		if ((Hardware::Get()->Micros() - m_QuikFindDiscovery.nMicros) > rdmdiscovery::RECEIVE_TIME_OUT) {
			assert(m_QuikFind.nCounter > 0);
			m_QuikFindDiscovery.nCounter--;
			m_QuikFindDiscovery.bCommandRunning = false;
		}

		return;
		break;
	case rdmdiscovery::State::FINISHED: ///< FINISHED
		m_bIsFinished = true;
		NEW_STATE(rdmdiscovery::State::IDLE, false);
#ifndef NDEBUG
		m_pRDMTod->Dump();

		printf("\nStack top %d\n\n", m_Discovery.stack.nDebugStackTopMax);

		for (uint32_t i = 0; i < debug.nTreeIndex; i++) {
			rdmdiscovery::print_uid(debug.tree[i].nLowerBound); printf(" "); rdmdiscovery::print_uid(debug.tree[i].nUpperBound); puts("");
		}
#endif
		break;
	default:
		assert(0);
		__builtin_unreachable();
		break;
	}
}
