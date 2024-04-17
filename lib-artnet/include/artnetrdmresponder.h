/**
 * @file artnetrdmresponder.h
 *
 */
/* Copyright (C) 2018-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef ARTNETRDMRESPONDER_H_
#define ARTNETRDMRESPONDER_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "rdmdeviceresponder.h"
#include "rdmpersonality.h"
#include "rdmhandler.h"
#include "rdmconst.h"
#include "rdm_message_print.h"

#include "lightset.h"

#include "debug.h"

#if defined (NODE_RDMNET_LLRP_ONLY)
# error "Cannot be both RDMNet Device and RDM Responder"
#endif

class ArtNetRdmResponder final: public RDMDeviceResponder, RDMHandler {
public:
	ArtNetRdmResponder(RDMPersonality **pRDMPersonalities, const uint32_t nPersonalityCount) :
		RDMDeviceResponder(pRDMPersonalities, nPersonalityCount)
	{
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	~ArtNetRdmResponder() {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	void TodCopy(const uint32_t nPortIndex, unsigned char *tod) {
		DEBUG_PRINTF("nPortIndex=%u", nPortIndex);

		if (nPortIndex == 0) {
			memcpy(tod, RDMDeviceResponder::GetUID(), RDM_UID_SIZE);
		} else {
			memcpy(tod, UID_ALL, RDM_UID_SIZE);
		}
	}

	const uint8_t *Handler(const uint32_t nPortIndex, const uint8_t *pRdmDataNoSC) {
		DEBUG_ENTRY

		if (nPortIndex != 0) {
			DEBUG_EXIT
			return nullptr;
		}

		if (pRdmDataNoSC == nullptr) {
			DEBUG_EXIT
			return nullptr;
		}

#ifndef NDEBUG
		rdm::message_print_no_sc(pRdmDataNoSC);
#endif

		RDMHandler::HandleData(pRdmDataNoSC, reinterpret_cast<uint8_t*>(&s_RdmCommand));

		if (s_RdmCommand.start_code != E120_SC_RDM) {
			DEBUG_EXIT
			return nullptr;
		}

#ifndef NDEBUG
		rdm::message_print(reinterpret_cast<uint8_t*>(&s_RdmCommand));
#endif

		DEBUG_EXIT
		return reinterpret_cast<const uint8_t*>(&s_RdmCommand);
	}

private:
	static TRdmMessage s_RdmCommand;
};

#endif /* ARTNETRDMRESPONDER_H_ */
