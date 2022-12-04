/**
 * @file rdm_send.cpp
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

#include <cstdint>
#include <cstring>
#include <cassert>

#include "artnetrdm.h"

#include "rdm.h"
#include "rdmmessage.h"
#include "rdm_e120.h"

#include "debug.h"

namespace artnet {

void rdm_send(uint32_t nPortIndex, const uint8_t *pRdmData) {
	assert(pRdmData != nullptr);

	auto *pRdmMessage = reinterpret_cast<const TRdmMessageNoSc *>(pRdmData);

	if ((pRdmMessage->command_class == E120_GET_COMMAND_RESPONSE) || (pRdmMessage->command_class == E120_SET_COMMAND_RESPONSE)) {
		struct TRdmMessage rdmMessage;

		rdmMessage.start_code = E120_SC_RDM;

		auto *pData = reinterpret_cast<uint8_t *>(&rdmMessage);

		memcpy(&pData[1], pRdmData, pRdmMessage->message_length - 1U);

		uint32_t i;
		uint16_t nChecksum = 0;

		for (i = 0; i < rdmMessage.message_length; i++) {
			nChecksum = static_cast<uint16_t>(nChecksum + pData[i]);
		}

		pData[i++] = static_cast<uint8_t>(nChecksum >> 8);
		pData[i] = static_cast<uint8_t>(nChecksum & 0XFF);

		Rdm::SendRaw(nPortIndex, reinterpret_cast<const uint8_t*>(&rdmMessage), rdmMessage.message_length + RDM_MESSAGE_CHECKSUM_SIZE);

#ifndef NDEBUG
		RDMMessage::Print(reinterpret_cast<const uint8_t *>(&rdmMessage));
#endif
	}
}

}  // namespace artnet
