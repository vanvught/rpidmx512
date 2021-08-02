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

#include "dmxset.h"
#include "dmxconst.h"

#if defined (H3)
#include "h3/dmx_config.h"
#elif defined(RPI1) || defined (RPI2)
#include "rpi/dmx_config.h"
#else
#include "linux/dmx_config.h"
#endif

struct Data {
	uint8_t data[dmx::buffer::SIZE];
	uint32_t nSlotsInPacket;
};

class DmxMulti: public DmxSet {
public:
	DmxMulti();

	void SetPortDirection(uint32_t nPort, dmx::PortDirection portDirection, bool bEnableData = false) override;

	// RDM Send

	void RdmSendRaw(uint32_t nPort, const uint8_t *pRdmData, uint32_t nLength) override;

	// RDM Receive

	const uint8_t *RdmReceive(uint32_t nPort) override;
	const uint8_t *RdmReceiveTimeOut(uint32_t nPort, uint32_t nTimeOut) override;

	 uint32_t RdmGetDateReceivedEnd() override {
		 return 0;
	 }

	// DMX Send

	void SetPortSendDataWithoutSC(uint32_t nPort, const uint8_t *pData, uint32_t nLength);

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

	// DMX Receive

	const uint8_t* GetDmxAvailable(uint32_t port);
	uint32_t GetUpdatesPerSeconde(uint32_t port);

	static void UartInit(uint32_t nUart);

private:
	void ClearData(uint32_t uart);
	void StartData(uint32_t uart);
	void StopData(uint32_t uart);

private:
	uint32_t m_nDmxTransmitBreakTime { dmx::transmit::BREAK_TIME_MIN };
	uint32_t m_nDmxTransmitMabTime { dmx::transmit::MAB_TIME_MIN };
	uint32_t m_nDmxTransmitPeriod { dmx::transmit::PERIOD_DEFAULT };
	uint32_t m_nDmxTransmitPeriodRequested { dmx::transmit::PERIOD_DEFAULT };
	dmx::PortDirection m_tDmxPortDirection[dmxmulti::max::OUT];
	uint32_t m_nDmxTransmissionLength[dmxmulti::max::OUT];
};

#endif /* H3_DMXMULTI_H_ */
