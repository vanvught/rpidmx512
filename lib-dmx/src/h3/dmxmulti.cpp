/**
 * @file dmxmulti.cpp
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

#include <cassert>

#include "h3.h"
#include "h3_hs_timer.h"
#include "h3/dmxmulti.h"
#include "h3/dmx_multi.h"

#include "dmxgpioparams.h"

#include "dmx_multi_internal.h"

#include "rdm.h"

#include "uart.h"

#include "debug.h"

DmxMulti::DmxMulti() {
	DEBUG_ENTRY

	dmx_multi_init();

	DEBUG_EXIT
}

void DmxMulti::SetPortDirection(uint8_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData) {
	assert(nPort < DMX_MAX_OUT);

	DEBUG_PRINTF("%d,%d,%d", nPort, tPortDirection, bEnableData);

	dmx_multi_set_port_direction(nPort, static_cast<_dmx_port_direction>(tPortDirection), bEnableData);
}

void DmxMulti::RdmSendRaw(uint8_t nPort, const uint8_t* pRdmData, uint16_t nLength) {
	assert(nPort < DMX_MAX_OUT);
	assert(pRdmData != nullptr);
	assert(nLength != 0);

	H3_UART_TypeDef *p = _get_uart(_port_to_uart(nPort));

	p->LCR = UART_LCR_8_N_2 | UART_LCR_BC;
	udelay(RDM_TRANSMIT_BREAK_TIME);

	p->LCR = UART_LCR_8_N_2;
	udelay(RDM_TRANSMIT_MAB_TIME);

	for (uint16_t i = 0; i < nLength; i++) {
		while (!(p->LSR & UART_LSR_THRE))
			;
		p->O00.THR = pRdmData[i];
	}

	while ((p->USR & UART_USR_BUSY) == UART_USR_BUSY) {
		static_cast<void>(p->O00.RBR);
	}
}

const uint8_t *DmxMulti::RdmReceive(uint8_t nPort) {
	assert(nPort < DMX_MAX_OUT);

	const uint8_t *p = dmx_multi_rdm_get_available(_port_to_uart(nPort));
	return p;
}

const uint8_t *DmxMulti::RdmReceiveTimeOut(uint8_t nPort, uint32_t nTimeOut) {
	assert(nPort < DMX_MAX_OUT);

	uint8_t *p = nullptr;
	const uint32_t nMicros = h3_hs_timer_lo_us() + nTimeOut;

	do {
		if ((p = const_cast<uint8_t*>(dmx_multi_rdm_get_available(_port_to_uart(nPort)))) != nullptr) {
			return p;
		}
	} while (h3_hs_timer_lo_us() < nMicros);

	return p;
}
