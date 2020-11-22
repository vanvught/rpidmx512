/**
 * @file rdmmessageprint.cpp
 *
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "rdmmessage.h"

#include "rdm.h"
#include "rdm_e120.h"

void RDMMessage::PrintNoSc(const uint8_t *pRdmDataNoSc) {
	const auto *pData = reinterpret_cast<const struct TRdmMessage*>(pRdmDataNoSc);

	uint8_t dummy[512];
	dummy[0] = E120_SC_RDM;
	memcpy(&dummy[1], pData, pData->message_length);

	Print(dummy);
}

void RDMMessage::Print(const uint8_t *pRdmData) {
	if (pRdmData == nullptr) {
		printf("No RDM data {pRdmData == 0}\n");
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
			printf("DISCOVERY_COMMAND, ");
			break;
		case E120_DISCOVERY_COMMAND_RESPONSE:
			printf("DISCOVERY_COMMAND_RESPONSE, ");
			break;
		case E120_GET_COMMAND:
			printf("GET_COMMAND, ");
			break;
		case E120_GET_COMMAND_RESPONSE:
			printf("GET_COMMAND_RESPONSE, ");
			break;
		case E120_SET_COMMAND:
			printf("SET_COMMAND, ");
			break;
		case E120_SET_COMMAND_RESPONSE:
			printf("SET_COMMAND_RESPONSE, ");
			break;
		default:
			printf("CC {%.2x}, ", pRdmMessage->command_class);
			break;
		}

		const uint16_t nSubDevice = (pRdmMessage->sub_device[0] << 8) + pRdmMessage->sub_device[1];

		printf("sub-dev: %d, tn: %d, PID 0x%.2x%.2x, pdl: %d\n", nSubDevice, pRdmMessage->transaction_number, pRdmMessage->param_id[0], pRdmMessage->param_id[1], pRdmMessage->param_data_length);

	} else if (pRdmData[0] == 0xFE) {
		for (uint32_t i = 0 ; i < 24; i++) {
			printf("%.2x ", pRdmData[i]);
		}
		printf("\n");
	} else {
		printf("Corrupted? RDM data [0-3]: %.2x:%.2x:%.2x:%.2x\n", pRdmData[0], pRdmData[1], pRdmData[2], pRdmData[3]);
	}
}
