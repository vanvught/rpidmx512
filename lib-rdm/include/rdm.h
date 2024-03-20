/**
 * @file rdm.h
 *
 */
/* Copyright (C) 2015-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDM_H_
#define RDM_H_

#include <cstdint>
#include <cassert>

#include "rdmconst.h"
#include "dmx.h"

#include "hal_api.h"

class Rdm {
public:
	static void SendRaw(const uint32_t nPortIndex, const uint8_t *pRdmData, const uint32_t nLength) {
		assert(pRdmData != nullptr);
		assert(nLength != 0);

		Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::OUTP, false);

		Dmx::Get()->RdmSendRaw(nPortIndex, pRdmData, nLength);

		udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);

		Dmx::Get()->SetPortDirection(nPortIndex, dmx::PortDirection::INP, true);
	}

	static void Send(const uint32_t nPortIndex, struct TRdmMessage *pRdmCommand) {
		assert(nPortIndex < dmx::config::max::PORTS);
		assert(pRdmCommand != nullptr);

		auto *pData = reinterpret_cast<uint8_t*>(pRdmCommand);
		uint32_t i;
		uint16_t nChecksum = 0;

		pRdmCommand->transaction_number = s_TransactionNumber[nPortIndex];

		for (i = 0; i < pRdmCommand->message_length; i++) {
			nChecksum = static_cast<uint16_t>(nChecksum + pData[i]);
		}

		pData[i++] = static_cast<uint8_t>(nChecksum >> 8);
		pData[i] = static_cast<uint8_t>(nChecksum & 0XFF);

		SendRaw(nPortIndex, reinterpret_cast<const uint8_t*>(pRdmCommand), pRdmCommand->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

		s_TransactionNumber[nPortIndex]++;
	}

	static void SendRawRespondMessage(const uint32_t nPortIndex, const uint8_t *pRdmData, const uint32_t nLength) {
		assert(nPortIndex < dmx::config::max::PORTS);
		assert(pRdmData != nullptr);
		assert(nLength != 0);

		extern volatile uint32_t gsv_RdmDataReceiveEnd;
		// 3.2.2 Responder Packet spacing
		udelay(RDM_RESPONDER_PACKET_SPACING, gsv_RdmDataReceiveEnd);

		SendRaw(nPortIndex, pRdmData, nLength);
	}

	static void SendDiscoveryRespondMessage(const uint32_t nPortIndex, const uint8_t *pRdmData, const uint32_t nLength) {
		 Dmx::Get()->RdmSendDiscoveryRespondMessage(nPortIndex, pRdmData, nLength);
	}

	static const uint8_t *Receive(const uint32_t nPortIndex) {
		return Dmx::Get()->RdmReceive(nPortIndex);
	}

	static const uint8_t *ReceiveTimeOut(const uint32_t nPortIndex, const uint16_t nTimeOut) {
		return Dmx::Get()->RdmReceiveTimeOut(nPortIndex, nTimeOut);
	}

private:
	static uint8_t s_TransactionNumber[dmx::config::max::PORTS];
};

#endif /* RDM_H_ */
