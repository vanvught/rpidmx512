/**
 * @file rdmmessageprint.cpp
 *
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>

#include "rdmmessage.h"

#include "rdm.h"
#include "rdm_e120.h"

void RDMMessage::Print(const uint8_t *pRdmData) {
	uint8_t *pUid;

	if (pRdmData == 0) {
		printf("No RDM data {pRdmData == 0}\n");
		return;
	}

	struct TRdmMessage *p = (struct TRdmMessage *) pRdmData;

	if (pRdmData[0] == E120_SC_RDM) {

		pUid = p->source_uid;
		printf("%.2x%.2x:%.2x%.2x%.2x%.2x -> ", pUid[0], pUid[1], pUid[2], pUid[3], pUid[4], pUid[5]);

		pUid = p->destination_uid;
		printf("%.2x%.2x:%.2x%.2x%.2x%.2x ", pUid[0], pUid[1], pUid[2], pUid[3], pUid[4], pUid[5]);

		switch (p->command_class) {
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
			printf("CC {%.2x}, ", p->command_class);
			break;
		}

		const uint16_t sub_device = (uint16_t) (p->sub_device[0] << 8) + (uint16_t) p->sub_device[1];

		printf("sub-dev: %d, tn: %d, PID 0x%.2x%.2x, pdl: %d\n", sub_device, p->transaction_number, p->param_id[0], p->param_id[1], p->param_data_length);

	} else if (pRdmData[0] == 0xFE) {
		for (uint8_t i = 0 ; i < 24; i++) {
			printf("%.2x ", pRdmData[i]);
		}
		printf("\n");
	} else {
		printf("Corrupted? RDM data [0-3]: %.2x:%.2x:%.2x:%.2x\n", pRdmData[0], pRdmData[1], pRdmData[2], pRdmData[3]);
	}
}
