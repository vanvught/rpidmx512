/**
 * @file dmx.h
 *
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RPI_DMX_H_
#define RPI_DMX_H_

#include <cstdint>

#include "dmxconst.h"
#include "dmx_config.h"

struct TotalStatistics {
	uint32_t nDmxPackets;
	uint32_t nRdmPackets;
};

struct Statistics {
	uint32_t nSlotsInPacket;
	uint32_t nSlotToSlot;
	uint32_t nMarkAfterBreak;
	uint32_t nBreakToBreak;
};

struct Data {
	uint8_t Data[dmx::buffer::SIZE];
	struct Statistics Statistics;
};

class Dmx {
public:
	Dmx();

	void SetPortDirection(uint32_t nPortIndex, dmx::PortDirection portDirection, bool bEnableData = false);
	dmx::PortDirection GetPortDirection(uint32_t nPortIndex);

	// RDM Send
	
	void RdmSendRaw(uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength);
	void RdmSendDiscoveryRespondMessage(const uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength);

	// RDM Receive

	const uint8_t *RdmReceive(uint32_t nPortIndex);
	const uint8_t *RdmReceiveTimeOut(uint32_t nPortIndex, uint32_t nTimeOut);

	// DMX Send
	
	void SetSendData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength);
	void SetSendDataWithoutSC(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength);

	void StartOutput(uint32_t nPortIndex);
	void SetOutput(const bool doForce);

	void SetOutputStyle(const uint32_t nPortIndex, const dmx::OutputStyle outputStyle) {}
	dmx::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const {
		return dmx::OutputStyle::CONTINOUS;
	}

	void Blackout();
	void FullOn();

	void ClearData(uint32_t nPortIndex);

	void SetDmxBreakTime(uint32_t nBreakTime);
	uint32_t GetDmxBreakTime();

	void SetDmxMabTime(uint32_t nMabTime);
	uint32_t GetDmxMabTime();

	void SetDmxPeriodTime(uint32_t nPeriodTime);
	uint32_t GetDmxPeriodTime();

	void SetDmxSlots(uint16_t nSlots = dmx::max::CHANNELS);
	uint16_t GetDmxSlots();

	uint32_t GetSendDataLength() ;

	const volatile struct TotalStatistics *GetTotalStatistics();

	// DMX Receive
	
	const uint8_t* GetDmxAvailable(uint32_t nPortIndex);
	uint32_t GetUpdatesPerSecond(uint32_t nPortIndex);

	const uint8_t* GetDmxCurrentData(uint32_t nPortIndex);
	const uint8_t* GetDmxChanged(uint32_t nPortIndex);

	static Dmx* Get() {
		return s_pThis;
	}

private:
	void StartData();
	void StopData();
	void SetSendDataLength(uint32_t nLength);
	void UartInit();
	void UartEnableFifo();
	void UartDisableFifo();

private:
	uint32_t m_nDmxTransmitPeriodRequested { dmx::transmit::PERIOD_DEFAULT };
	uint8_t m_nDataDirectionGpio { GPIO_DMX_DATA_DIRECTION };

	static Dmx *s_pThis;
};

#endif /* RPI_DMX_H_ */
