/**
 * @file e131controller.h
 *
 */
/* Copyright (C) 2020-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef E131CONTROLLER_H_
#define E131CONTROLLER_H_

#include <cstdint>

#include "e131.h"
#include "e131packets.h"

enum {
	DEFAULT_SYNCHRONIZATION_ADDRESS = 5000
};

#ifndef DMX_MAX_VALUE
#define DMX_MAX_VALUE 255
#endif

struct TE131ControllerState {
	bool bIsRunning;
	uint16_t nActiveUniverses;
	uint32_t DiscoveryTime;
	uint8_t nPriority;
	struct TSynchronizationPacket {
		uint16_t nUniverseNumber;
		uint32_t nIpAddress;
		uint8_t nSequenceNumber;
	} SynchronizationPacket;
};

class E131Controller {
public:
	E131Controller();
	~E131Controller();

	void Start();
	void Stop();
	void Run();

	void Print();

	void HandleDmxOut(uint16_t nUniverse, const uint8_t *pDmxData, uint32_t nLength);
	void HandleSync();
	void HandleBlackout();

	void SetSynchronizationAddress(uint16_t nSynchronizationAddress = DEFAULT_SYNCHRONIZATION_ADDRESS) {
		m_State.SynchronizationPacket.nUniverseNumber = nSynchronizationAddress;
		m_State.SynchronizationPacket.nIpAddress = e131::universe_to_multicast_ip(nSynchronizationAddress);
	}
	uint16_t GetSynchronizationAddress() const {
		return m_State.SynchronizationPacket.nUniverseNumber;
	}

	void SetMaster(uint32_t nMaster = DMX_MAX_VALUE) {
		if (nMaster < DMX_MAX_VALUE) {
			m_nMaster = nMaster;
		} else {
			m_nMaster = DMX_MAX_VALUE;
		}
	}
	uint32_t GetMaster() const {
		return m_nMaster;
	}

	const uint8_t *GetSoftwareVersion();

	void SetSourceName(const char *pSourceName);
	void SetPriority(uint8_t nPriority);

	static E131Controller* Get() {
		return s_pThis;
	}

private:
	void FillDataPacket();
	void FillDiscoveryPacket();
	void FillSynchronizationPacket();
	void SendDiscoveryPacket();
	uint8_t GetSequenceNumber(uint16_t nUniverse, uint32_t &nMulticastIpAddress);

private:
	int32_t m_nHandle { -1 };
	uint32_t m_nCurrentPacketMillis { 0 };
	struct TE131ControllerState m_State;
	TE131DataPacket *m_pE131DataPacket { nullptr };
	TE131DiscoveryPacket *m_pE131DiscoveryPacket { nullptr };
	TE131SynchronizationPacket *m_pE131SynchronizationPacket { nullptr };
	uint32_t m_DiscoveryIpAddress { 0 };
	uint8_t m_Cid[e131::CID_LENGTH];
	char m_SourceName[e131::SOURCE_NAME_LENGTH];
	uint32_t m_nMaster { DMX_MAX_VALUE };

	static E131Controller *s_pThis;
};

#endif /* E131CONTROLLER_H_ */
