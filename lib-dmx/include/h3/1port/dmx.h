/**
 * @file dmx.h
 *
 */
/* Copyright (C) 2015-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef H3_DMX_H_
#define H3_DMX_H_

#include <cstdint>

#include "dmxconst.h"

#include "../dmx_config.h"

namespace dmxsingle {
namespace max {
static constexpr auto OUT = 1U;
static constexpr auto IN = 1U;
}  // namespace max

struct TotalStatistics {
	uint32_t nDmxPackets;
	uint32_t nRdmPackets;
};

struct Statistics {
	uint32_t nSlotsInPacket;
	uint32_t nMarkAfterBreak;
	uint32_t nBreakToBreak;
	uint32_t nSlotToSlot;
};

struct Data {
	uint8_t Data[dmx::buffer::SIZE];
	struct Statistics Statistics;
};
}  // namespace dmxsingle

class Dmx {
public:
	Dmx(bool DoInit = true);
	void Init();

	void SetPortDirection(uint32_t nPortIndex, dmx::PortDirection portDirection, bool bEnableData = false);
	dmx::PortDirection GetPortDirection();

	// RDM
	void RdmSendRaw(uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength);
	const uint8_t *RdmReceive(uint32_t nPortIndex);
	const uint8_t *RdmReceiveTimeOut(uint32_t nPortIndex, uint16_t nTimeOut);
	uint32_t RdmGetDateReceivedEnd();

	// DMX
	void ClearData();
	void SetSendData(const uint8_t *pData, uint32_t nLength);
	void SetSendDataWithoutSC(const uint8_t *pData, uint32_t nLength);	//TODO Remove void SetSendDataWithoutSC(const uint8_t *pData, uint32_t nLength)
	void SetPortSendDataWithoutSC(__attribute__((unused))uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength) {
		SetSendDataWithoutSC(pData, nLength);
	}

	const uint8_t* GetDmxCurrentData();
	const uint8_t* GetDmxAvailable();
	const uint8_t* GetDmxChanged();
	uint32_t GetUpdatesPerSecond();

	void SetDmxBreakTime(uint32_t nBreakTime);
	uint32_t GetDmxBreakTime() const {
		return m_nDmxTransmitBreakTime;
	}

	void SetDmxMabTime(uint32_t nMabTime);
	uint32_t GetDmxMabTime() const {
		return m_nDmxTransmitMabTime;
	}

	void SetDmxPeriodTime(uint32_t nPeriodTime);
	uint32_t GetDmxPeriodTime() const {
		return m_nDmxTransmitPeriod;
	}

	void SetDmxSlots(uint16_t nSlots = dmx::max::CHANNELS);
	uint16_t GetDmxSlots() const {
		return m_nDmxTransmitSlots;
	}

	static Dmx* Get() {
		return s_pThis;
	}

private:
	void UartInit();
	void SetSendDataLength(uint32_t nLength);
	void UartEnableFifo();
	void UartDisableFifo();
	void StopData();
	void StartData();

private:
	bool m_IsInitDone { false };
	uint32_t m_nDmxTransmitBreakTime { dmx::transmit::BREAK_TIME_MIN };
	uint32_t m_nDmxTransmitMabTime { dmx::transmit::MAB_TIME_MIN };
	uint32_t m_nDmxTransmitPeriod = { dmx::transmit::PERIOD_DEFAULT };
	uint32_t m_nDmxTransmitPeriodRequested { dmx::transmit::PERIOD_DEFAULT };
	uint16_t m_nDmxTransmitSlots { dmx::max::CHANNELS };

	static Dmx *s_pThis;
};

#endif /* H3_DMX_H_ */