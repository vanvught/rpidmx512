/**
 * @file dmx++.cpp
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "dmx.h"
#include "rdm.h"

#include "h3_timer.h"
#include "h3_board.h"

#include "uart.h"

const uint8_t *Dmx::RdmReceive(__attribute__((unused)) uint32_t nPort) {
	assert(nPort == 0);

	const auto *p = rdm_get_available();
	return p;
}

const uint8_t* Dmx::RdmReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut) {
	assert(nPort == 0);

	uint8_t *p = nullptr;
	const auto nMicros = H3_TIMER->AVS_CNT1;

	do {
		if ((p = const_cast<uint8_t*>(RdmReceive(nPort))) != nullptr) {
			return p;
		}
	} while ((H3_TIMER->AVS_CNT1 - nMicros) < nTimeOut);

	return p;
}

void Dmx::SetPortDirection(__attribute__((unused)) uint32_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData) {
	assert(nPort == 0);

	dmx_set_port_direction(static_cast<_dmx_port_direction>(tPortDirection), bEnableData);
}

void Dmx::RdmSendRaw(__attribute__((unused)) uint32_t nPort, const uint8_t *pRdmData, uint16_t nLength) {
	assert(nPort == 0);

	while (!(EXT_UART->LSR & UART_LSR_TEMT))
		;

	EXT_UART->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
	udelay(RDM_TRANSMIT_BREAK_TIME);

	EXT_UART->LCR = UART_LCR_8_N_2;
	udelay(RDM_TRANSMIT_MAB_TIME);

	for (uint16_t i = 0; i < nLength; i++) {
		while (!(EXT_UART->LSR & UART_LSR_THRE))
			;
		EXT_UART->O00.THR = pRdmData[i];
	}

	while ((EXT_UART->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		(void) EXT_UART->O00.RBR;
	}
}
