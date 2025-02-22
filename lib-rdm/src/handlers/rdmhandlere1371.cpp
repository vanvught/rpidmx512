/**
 * @file rdmhandlere1371.cpp
 *
 */
/* Copyright (C) 2020-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstring>

#include "rdmhandler.h"
#include "rdmconst.h"
#include "rdmidentify.h"
#include "rdm_e120.h"

#include "debug.h"

/*
 * ANSI E1.37-1
 */

void RDMHandler::GetIdentifyMode([[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);

	pRdmDataOut->param_data_length = 1;
	pRdmDataOut->param_data[0] = static_cast<uint8_t>(RDMIdentify::Get()->GetMode());

	RespondMessageAck();

	DEBUG_EXIT
}

void RDMHandler::SetIdentifyMode(bool IsBroadcast, [[maybe_unused]] uint16_t nSubDevice) {
	DEBUG_ENTRY

	const auto *pRdmDataIn = reinterpret_cast<const struct TRdmMessageNoSc*>(m_pRdmDataIn);

	if (pRdmDataIn->param_data_length != 1) {
		RespondMessageNack(E120_NR_FORMAT_ERROR);
		DEBUG_EXIT
		return;
	}

	if ((pRdmDataIn->param_data[0] != 0) && (pRdmDataIn->param_data[0] != 0xFF)) {
		RespondMessageNack( E120_NR_DATA_OUT_OF_RANGE);
		DEBUG_EXIT
		return;
	}

	RDMIdentify::Get()->SetMode(static_cast<rdm::identify::Mode>(pRdmDataIn->param_data[0]));

	if (IsBroadcast) {
		DEBUG_EXIT
		return;
	}

	auto *pRdmDataOut = reinterpret_cast<struct TRdmMessage*>(m_pRdmDataOut);
	pRdmDataOut->param_data_length = 0;

	RespondMessageAck();

	DEBUG_EXIT
}
