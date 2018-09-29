/**
 * @file rdmmessage.cpp
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

#if defined (BARE_METAL)
 #include "util.h"
#elif defined(__circle__)
 #include "circle/util.h"
#else
 #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

#include "rdmmessage.h"

#include "rdm.h"
#include "rdm_e120.h"

RDMMessage::RDMMessage(void)  {
	m_pRdmCommand = new struct TRdmMessage;

	m_pRdmCommand->start_code = E120_SC_RDM;
	m_pRdmCommand->sub_start_code = E120_SC_SUB_MESSAGE;
	m_pRdmCommand->message_length = RDM_MESSAGE_MINIMUM_SIZE;
	memcpy(m_pRdmCommand->source_uid, UID_ALL, RDM_UID_SIZE);
	memcpy(m_pRdmCommand->destination_uid, UID_ALL, RDM_UID_SIZE);
	//m_pRdmCommand->transaction_number = m_TransactionNumber;
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

void RDMMessage::Send(uint8_t nPort) {
	Rdm::Send(nPort, m_pRdmCommand);
}
