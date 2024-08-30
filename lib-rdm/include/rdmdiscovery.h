/**
 * @file rdmddiscovery.h
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

#ifndef RDMDDISCOVERY_H_
#define RDMDDISCOVERY_H_

#include <rdmtod.h>
#include <cstdint>
#include <algorithm>

#include "rdmmessage.h"
#include "debug.h"

namespace rdmdiscovery {
#ifndef NDEBUG
 static constexpr uint32_t RECEIVE_TIME_OUT = 58000;
 static constexpr uint32_t LATE_RESPONSE_TIME_OUT = 40000;
#else
 static constexpr uint32_t RECEIVE_TIME_OUT = 5800;
 static constexpr uint32_t LATE_RESPONSE_TIME_OUT = 1000;
#endif
static constexpr uint32_t UNMUTE_COUNTER = 3;
static constexpr uint32_t MUTE_COUNTER = 10;
static constexpr uint32_t DISCOVERY_STACK_SIZE = rdmtod::TOD_TABLE_SIZE;
static constexpr uint32_t DISCOVERY_COUNTER = 3;
static constexpr uint32_t QUIKFIND_COUNTER = 5;
static constexpr uint32_t QUIKFIND_DISCOVERY_COUNTER = 5;

enum class State {
	IDLE,
	UNMUTE,
	MUTE,
	DISCOVERY,
	DISCOVERY_SINGLE_DEVICE,
	DUB,
	QUICKFIND,
	QUICKFIND_DISCOVERY,
	LATE_RESPONSE,
	FINISHED
};
}  // namespace rdmdiscovery

class RDMDiscovery {
public:
	RDMDiscovery(const uint8_t *pUid);

	bool Full(const uint32_t nPortIndex, RDMTod *pRDMTod);
	bool Incremental(const uint32_t nPortIndex, RDMTod *pRDMTod);

	bool Stop();

	bool IsRunning(uint32_t& nPortIndex, bool& bIsIncremental) const {
		nPortIndex = m_nPortIndex;
		bIsIncremental = m_doIncremental;
		return (m_State != rdmdiscovery::State::IDLE);
	}

	bool IsFinished(uint32_t& nPortIndex, bool& bIsIncremental) {
		nPortIndex = m_nPortIndex;
		bIsIncremental = m_doIncremental;

		if (m_bIsFinished) {
			m_bIsFinished = false;
			return true;
		}

		return false;
	}

	uint32_t CopyWorkingQueue(char *pOutBuffer, const uint32_t nOutBufferSize);

	void Run() {
		if (__builtin_expect((m_State == rdmdiscovery::State::IDLE), 1)) {
			return;
		}

		Process();
	}

private:
	void Process();
	bool Start(const uint32_t nPortIndex, RDMTod *pRDMTod, const bool doIncremental);
	bool IsValidDiscoveryResponse(uint8_t *pUid);

	void SavedState([[maybe_unused]] const uint32_t nLine);
	void NewState(const rdmdiscovery::State state, const bool doStateLateResponse, [[maybe_unused]] const uint32_t nLine);

private:
	RDMMessage m_Message;
	uint8_t *m_pResponse { nullptr };
	uint8_t m_Uid[RDM_UID_SIZE];
	uint32_t m_nPortIndex { 0 };
	RDMTod *m_pRDMTod { nullptr };

	bool m_bIsFinished { false };
	bool m_doIncremental { false };
	rdmdiscovery::State m_State { rdmdiscovery::State::IDLE };
	rdmdiscovery::State m_SavedState { rdmdiscovery::State::IDLE };

	struct {
		uint32_t nMicros;
	} m_LateResponse;

	struct {
		uint32_t nCounter;
		uint32_t nMicros;
		bool bCommandRunning;
	} m_UnMute;

	struct {
		uint32_t nTodEntries;
		uint32_t nCounter;
		uint32_t nMicros;
		uint8_t uid[RDM_UID_SIZE];
		bool bCommandRunning;
	} m_Mute;

	struct {
		struct {
			bool push(const uint64_t nLowerBound, const uint64_t nUpperBound) {
				if (nTop == rdmdiscovery::DISCOVERY_STACK_SIZE - 1) {
					assert(0);
					return false;
				}

				nTop++;
				items[nTop].nLowerBound = nLowerBound;
				items[nTop].nUpperBound = nUpperBound;

				nDebugStackTopMax = std::max(nDebugStackTopMax, nTop);
				return true;
			}

			bool pop(uint64_t &nLowerBound, uint64_t &nUpperBound) {
				if (nTop == -1) {
					return false;
				}

				nLowerBound = items[nTop].nLowerBound;
				nUpperBound = items[nTop].nUpperBound;
				nTop--;

				return true;
			}

			int32_t nTop;

			struct {
				uint64_t nLowerBound;
				uint64_t nUpperBound;
			} items[rdmdiscovery::DISCOVERY_STACK_SIZE];

			int32_t nDebugStackTopMax;
		} stack;

		uint64_t nLowerBound;
		uint64_t nMidPosition;
		uint64_t nUpperBound;
		uint32_t nCounter;
		uint32_t nMicros;
		uint8_t uid[RDM_UID_SIZE];
		uint8_t pdl[2][RDM_UID_SIZE];
		bool bCommandRunning;
	} m_Discovery;

	struct {
		uint32_t nCounter;
		uint32_t nMicros;
		bool bCommandRunning;
	} m_DiscoverySingleDevice;

	struct {
		uint32_t nCounter;
		uint32_t nMicros;
		bool bCommandRunning;
		uint8_t uid[RDM_UID_SIZE];
	} m_QuikFind;

	struct {
		uint32_t nCounter;
		uint32_t nMicros;
		bool bCommandRunning;
		uint8_t uid[RDM_UID_SIZE];
	} m_QuikFindDiscovery;

#ifndef NDEBUG
	struct {
		struct {
			uint64_t nLowerBound;
			uint64_t nUpperBound;
		} tree[1024];

		uint32_t nTreeIndex;
	} debug;
#endif
};

#endif /* RDMDDISCOVERY_H_ */
