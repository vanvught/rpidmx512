/**
 * @file rdmqueuedmessage.cpp
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
#include <cassert>

#include "rdmqueuedmessage.h"
#include "rdmconst.h"
#include "rdm_e120.h"

RDMQueuedMessage::RDMQueuedMessage() {
	m_pQueue = new TRdmQueuedMessage[RDM_MESSAGE_COUNT_MAX];
	assert(m_pQueue != nullptr);
}

RDMQueuedMessage::~RDMQueuedMessage() {
	assert(m_pQueue == nullptr);
	delete[] m_pQueue;
}

void RDMQueuedMessage::Copy(struct TRdmMessage *pRdmMessage, uint32_t nIndex) {
	assert(nIndex < RDM_MESSAGE_COUNT_MAX);

	pRdmMessage->command_class = m_pQueue[nIndex].command_class;
	pRdmMessage->param_id[0] = m_pQueue[nIndex].param_id[0];
	pRdmMessage->param_id[1] = m_pQueue[nIndex].param_id[1];
	pRdmMessage->param_data_length = m_pQueue[nIndex].param_data_length;

	for (uint32_t i = 0; i < pRdmMessage->param_data_length; i++) {
		pRdmMessage->param_data[i] = m_pQueue[nIndex].param_data[i];
	}

}

uint8_t RDMQueuedMessage::GetMessageCount() const {
	return m_nMessageCount;
}

void RDMQueuedMessage::Handler(uint8_t *pRdmData) {
	auto rdm_response = reinterpret_cast<struct TRdmMessage*>(pRdmData);

	if (m_IsNeverQueued) {
		rdm_response->slot16.response_type = E120_STATUS_MESSAGES;
		rdm_response->param_data_length = 0;
	} else if (rdm_response->param_data[0] == E120_STATUS_GET_LAST_MESSAGE) {
		Copy(rdm_response, m_nMessageCount);
	} else {
		if (m_nMessageCount != 0) {
			m_nMessageCount--;
			Copy(rdm_response, m_nMessageCount);
		} else {
			rdm_response->slot16.response_type = E120_STATUS_MESSAGES;
			rdm_response->param_data_length = 0;
		}
	}
}

bool RDMQueuedMessage::Add(const struct TRdmQueuedMessage *pMessage) {
	m_IsNeverQueued = false;

	if (m_nMessageCount != RDM_MESSAGE_COUNT_MAX) {

		m_pQueue[m_nMessageCount].command_class = pMessage->command_class;
		m_pQueue[m_nMessageCount].param_id[0] = pMessage->param_id[0];
		m_pQueue[m_nMessageCount].param_id[1] = pMessage->param_id[1];
		m_pQueue[m_nMessageCount].param_data_length = pMessage->param_data_length;

		for (uint32_t i = 0; i < pMessage->param_data_length; i++) {
			m_pQueue[m_nMessageCount].param_data[i] = pMessage->param_data[i];
		}

		m_nMessageCount++;

		return true;
	}

	return false;
}
