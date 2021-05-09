/**
 * @file dmxmulti.h
 *
 */
/* Copyright (C) 2018-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef H3_DMXMULTI_H_
#define H3_DMXMULTI_H_

#include <cstdint>

#include "dmx.h"

class DmxMulti: public DmxSet {
public:
	DmxMulti();

	void SetPortDirection(uint32_t nPort, TDmxRdmPortDirection tPortDirection, bool bEnableData = false) override;

	void RdmSendRaw(uint32_t nPort, const uint8_t *pRdmData, uint16_t nLength) override;

	const uint8_t *RdmReceive(uint32_t nPort) override;
	const uint8_t *RdmReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut) override;

	void SetPortSendDataWithoutSC(uint32_t nPort, const uint8_t *pData, uint16_t nLength);

	void SetDmxBreakTime(uint32_t nBreakTime);
	uint32_t GetDmxBreakTime() const {
		return m_nDmxTransmitMabTime;
	}

	void SetDmxMabTime(uint32_t nMabTime);
	uint32_t GetDmxMabTime() const {
		return m_nDmxTransmitMabTime;
	}
	
	void SetDmxPeriodTime(uint32_t nPeriod);
	uint32_t GetDmxPeriodTime() const {
		return m_nDmxTransmitPeriod;
	}

	static void UartInit(uint32_t nUart);

private:
	void ClearData(uint32_t uart);
	void UartEnableFifoTx(uint32_t uart);
	void UartEnableFifoRx(uint32_t uart);
	void StartData(uint32_t uart);
	void StopData(uint32_t uart);

private:
	uint32_t m_nDmxTransmitBreakTime { DMX_TRANSMIT_BREAK_TIME_MIN };
	uint32_t m_nDmxTransmitMabTime { DMX_TRANSMIT_MAB_TIME_MIN };
	uint32_t m_nDmxTransmitPeriod { DMX_TRANSMIT_PERIOD_DEFAULT };
	uint32_t m_nDmxTransmitPeriodRequested { DMX_TRANSMIT_PERIOD_DEFAULT };
	uint8_t m_nDmxDataDirectionGpioPin[DMX_MAX_OUT];
	TDmxRdmPortDirection m_tDmxPortDirection[DMX_MAX_OUT];
	uint32_t m_nDmxTransmissionLength[DMX_MAX_OUT];
};

#endif /* H3_DMXMULTI_H_ */
