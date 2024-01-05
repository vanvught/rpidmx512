/**
 * @file rdmmessage.h
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

#ifndef RDMMESSAGE_H_
#define RDMMESSAGE_H_

#include <cstring>
#include <cassert>

#include "rdm_message_print.h"
#include "rdm.h"
#include "rdm_e120.h"

class RDMMessage final: public Rdm {
public:
	RDMMessage()  {
		m_message.start_code = E120_SC_RDM;
		m_message.sub_start_code = E120_SC_SUB_MESSAGE;
		m_message.message_length = RDM_MESSAGE_MINIMUM_SIZE;
		memcpy(m_message.source_uid, UID_ALL, RDM_UID_SIZE);
		memcpy(m_message.destination_uid, UID_ALL, RDM_UID_SIZE);
		m_message.slot16.port_id = 1;
		m_message.message_count = 0;
		m_message.sub_device[0] = 0;
		m_message.sub_device[1] = 0;
		m_message.param_data_length = 0;
	}

	void SetPortID(const uint8_t nPortID) {
		assert(nPortID > 0);
		m_message.slot16.port_id = nPortID;
	}

	void SetSrcUid(const uint8_t *SrcUid){
		memcpy(m_message.source_uid, SrcUid, RDM_UID_SIZE);
	}

	void SetDstUid(const uint8_t *DstUid){
		memcpy(m_message.destination_uid, DstUid, RDM_UID_SIZE);
	}

	void SetSubDevice(const uint16_t nSubDevice) {
		m_message.sub_device[0] = static_cast<uint8_t>(nSubDevice >> 8);
		m_message.sub_device[1] = static_cast<uint8_t>(nSubDevice);
	}

	void SetCc(const uint8_t nCc) {
		m_message.command_class = nCc;
	}

	void SetPid(const uint16_t nPid) {
		m_message.param_id[0] = static_cast<uint8_t>(nPid >> 8);
		m_message.param_id[1] = static_cast<uint8_t>(nPid);
	}

	void SetPd(const uint8_t *pParamData, const uint8_t nLength) {
		m_message.message_length = static_cast<uint8_t>(m_message.message_length - m_message.param_data_length);
		m_message.param_data_length = nLength;
		if ((pParamData != nullptr) && (nLength != 0)) {
			memcpy(m_message.param_data, pParamData, nLength);
		}
		m_message.message_length = static_cast<uint8_t>(m_message.message_length + nLength);
	}

	void Send(const uint32_t nPortIndex) {
#ifndef NDEBUG
		rdm::message_print(reinterpret_cast<const uint8_t *>(&m_message));
#endif
		Rdm::Send(nPortIndex, &m_message);
	}

private:
	TRdmMessage m_message;
};

#endif /* RDMMESSAGE_H_ */
