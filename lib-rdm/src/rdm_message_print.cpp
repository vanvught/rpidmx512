/**
 * @file rdm_message_print.cpp
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
#include <cstdio>
#include <cassert>

#include "rdmconst.h"
#include "rdm_e120.h"

#include "debug.h"

namespace rdm {
void message_print(const uint8_t *pRdmData) {
	if (pRdmData == nullptr) {
		DEBUG_PUTS("No RDM data");
		return;
	}

	const auto *pRdmMessage = reinterpret_cast<const struct TRdmMessage*>(pRdmData);

	if (pRdmData[0] == E120_SC_RDM) {
		const uint8_t *pSrcUid = pRdmMessage->source_uid;
		printf("%.2x%.2x:%.2x%.2x%.2x%.2x -> ", pSrcUid[0], pSrcUid[1], pSrcUid[2], pSrcUid[3], pSrcUid[4], pSrcUid[5]);

		const uint8_t *pDstUid = pRdmMessage->destination_uid;
		printf("%.2x%.2x:%.2x%.2x%.2x%.2x ", pDstUid[0], pDstUid[1], pDstUid[2], pDstUid[3], pDstUid[4], pDstUid[5]);

		switch (pRdmMessage->command_class) {
		case E120_DISCOVERY_COMMAND:
			printf("DISCOVERY_COMMAND");
			break;
		case E120_DISCOVERY_COMMAND_RESPONSE:
			printf("DISCOVERY_COMMAND_RESPONSE");
			break;
		case E120_GET_COMMAND:
			printf("GET_COMMAND");
			break;
		case E120_GET_COMMAND_RESPONSE:
			printf("GET_COMMAND_RESPONSE");
			break;
		case E120_SET_COMMAND:
			printf("SET_COMMAND");
			break;
		case E120_SET_COMMAND_RESPONSE:
			printf("SET_COMMAND_RESPONSE");
			break;
		default:
			printf("CC {%.2x}", pRdmMessage->command_class);
			break;
		}

		const auto nSubDevice = static_cast<uint16_t>((pRdmMessage->sub_device[0] << 8) + pRdmMessage->sub_device[1]);
		printf(", sub-dev: %d, tn: %d, PID 0x%.2x%.2x, pdl: %d", nSubDevice, pRdmMessage->transaction_number, pRdmMessage->param_id[0], pRdmMessage->param_id[1], pRdmMessage->param_data_length);

		if (pRdmMessage->param_data_length != 0) {
			printf(" -> ");
			for (uint32_t i = 0 ; (i < 12) && (i < pRdmMessage->param_data_length) ; i++) {
				printf("%.2x ", pRdmMessage->param_data[i]);
			}
		}

		puts("");

	} else if (pRdmData[0] == 0xFE) {
		for (uint32_t i = 0 ; i < 24; i++) {
			printf("%.2x ", pRdmData[i]);
		}
		puts("");
	} else {
		printf("Corrupted? RDM data [0-3]: %.2x:%.2x:%.2x:%.2x\n", pRdmData[0], pRdmData[1], pRdmData[2], pRdmData[3]);
	}
}

void message_print_no_sc(const uint8_t *pRdmDataNoSc) {
	assert(pRdmDataNoSc != nullptr);

	const auto *pData = reinterpret_cast<const struct TRdmMessageNoSc *>(pRdmDataNoSc);

	uint8_t message[sizeof(struct TRdmMessage)];
	message[0] = E120_SC_RDM;

	memcpy(&message[1], pData, pData->message_length - 1U);

	message_print(message);
}
}  // namespace rdm

