/**
 * @file rdmmessage.cpp
 *
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

//#define DEBUG

#include <stdint.h>
#include <stdio.h>

#include "rdmmessage.h"

#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"
#include "rdm_send.h"

#include "util.h"
#include "hardware.h"

uint8_t RDMMessage::m_TransactionNumber = 0;

RDMMessage::RDMMessage(void)  {
	m_pRdmCommand = new _rdm_command;

	m_pRdmCommand->start_code = E120_SC_RDM;
	m_pRdmCommand->sub_start_code = E120_SC_SUB_MESSAGE;
	m_pRdmCommand->message_length = RDM_MESSAGE_MINIMUM_SIZE;
	memcpy(m_pRdmCommand->source_uid, UID_ALL, RDM_UID_SIZE);
	memcpy(m_pRdmCommand->destination_uid, UID_ALL, RDM_UID_SIZE);
	m_pRdmCommand->transaction_number = m_TransactionNumber;
	m_pRdmCommand->slot16.port_id = 1;
	m_pRdmCommand->message_count = 0;
	m_pRdmCommand->sub_device[0] = 0;
	m_pRdmCommand->sub_device[1] = 0;
	m_pRdmCommand->param_data_length = 0;
}

RDMMessage::~RDMMessage(void) {
	delete m_pRdmCommand;
}

void RDMMessage::SetSrcUid(const uint8_t *SrcUid){
	memcpy(m_pRdmCommand->source_uid, SrcUid, RDM_UID_SIZE);
}

void RDMMessage::SetDstUid(const uint8_t *DstUid){
	memcpy(m_pRdmCommand->destination_uid, DstUid, RDM_UID_SIZE);
}

void RDMMessage::Send(void) {
	uint8_t *rdm_data = (uint8_t *)m_pRdmCommand;
	uint8_t i;
	uint16_t rdm_checksum = 0;

	m_pRdmCommand->transaction_number = m_TransactionNumber;

	for (i = 0; i < m_pRdmCommand->message_length; i++) {
		rdm_checksum += rdm_data[i];
	}

	rdm_data[i++] = rdm_checksum >> 8;
	rdm_data[i] = rdm_checksum & 0XFF;

#ifdef DEBUG
	Print(rdm_data);
#endif

	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);

	rdm_send_data((const uint8_t *)m_pRdmCommand, m_pRdmCommand->message_length + RDM_MESSAGE_CHECKSUM_SIZE);

	udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);
	dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);

	m_TransactionNumber++;
}

void RDMMessage::SetSubDevice(const uint16_t SubDevice) {
	m_pRdmCommand->sub_device[0] = (uint8_t) (SubDevice >> 8);
	m_pRdmCommand->sub_device[1] = (uint8_t) SubDevice;
}

void RDMMessage::SetCc(const uint8_t cc) {
	m_pRdmCommand->command_class = cc;
}

void RDMMessage::SetPid(const uint16_t pid) {
	m_pRdmCommand->param_id[0] = (uint8_t) (pid >> 8);
	m_pRdmCommand->param_id[1] = (uint8_t) pid;
}

void RDMMessage::SetPd(const uint8_t *pd, const uint8_t length) {
	m_pRdmCommand->message_length -= m_pRdmCommand->param_data_length;
	m_pRdmCommand->param_data_length = length;
	memcpy(m_pRdmCommand->param_data, pd, length);
	m_pRdmCommand->message_length += length;

}

const uint8_t *RDMMessage::Receive(void) {
	const uint8_t *p = rdm_get_available();

#ifdef DEBUG
	if (p != NULL) {
		printf("In  ");
		Print(p);
	}
#endif

	return p;
}

const uint8_t *RDMMessage::ReceiveTimeOut(const uint32_t nTimeOut) {
	uint8_t *p = NULL;
	uint32_t micros_now = BCM2835_ST->CLO;

	do {
		if ((p = (uint8_t *)rdm_get_available()) != NULL) {
#ifdef DEBUG
			Print(p);
#endif
			return (const uint8_t *) p;
		}
	} while ( BCM2835_ST->CLO - micros_now < nTimeOut);

	return (const uint8_t *) p;
}

void RDMMessage::Print(const uint8_t *rdm_data) {
	struct _rdm_command *p;

	if (rdm_data == NULL) {
		printf("No RDM data\n");
		return;
	}

	p = (struct _rdm_command *) (rdm_data);

	if (rdm_data[0] == E120_SC_RDM) {
		const uint8_t *uid = p->source_uid;
		printf("%.2x%.2x:%.2x%.2x%.2x%.2x -> ", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5]);
		uid = p->destination_uid;
		printf("%.2x%.2x:%.2x%.2x%.2x%.2x ", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5]);

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

		uint16_t sub_device = 0;
		printf("sub-dev: %d, tn: %d, PID 0x%.2x%.2x, pdl: %d\n", sub_device, p->transaction_number, p->param_id[0], p->param_id[1], p->param_data_length);

	} else if (rdm_data[0] == 0xFE) {
		for (unsigned i = 0 ; i < 24; i++) {
			printf("%.2x ", rdm_data[i]);
		}
		printf("\n");
	}
}

void RDMMessage::SendRaw(const uint8_t *pRdmData, const uint8_t nLength) {
	if ((pRdmData != 0) && (nLength != 0)) {
		dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);
		rdm_send_data((const uint8_t *) pRdmData, nLength);
		udelay(RDM_RESPONDER_DATA_DIRECTION_DELAY);
		dmx_set_port_direction(DMX_PORT_DIRECTION_INP, true);
	}
}
