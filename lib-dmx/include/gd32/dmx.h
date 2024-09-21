/**
 * @file dmx.h
 *
 */
/* Copyright (C) 2021-2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef GD32_DMX_H_
#define GD32_DMX_H_

#include <cstdint>

#include "dmxconst.h"
#include "dmx_config.h"
#include "dmxstatistics.h"

struct Statistics {
	uint32_t nSlotsInPacket;
};

struct Data {
	uint8_t Data[dmx::buffer::SIZE];
	struct Statistics Statistics;
};

class Dmx {
public:
	Dmx();

	void SetPortDirection(const uint32_t nPortIndex, const dmx::PortDirection portDirection, const bool bEnableData = false);
	dmx::PortDirection GetPortDirection(const uint32_t nPortIndex) const {
		return m_dmxPortDirection[nPortIndex];
	}

	void ClearData(const uint32_t nPortIndex);

	volatile dmx::TotalStatistics& GetTotalStatistics(const uint32_t nPortIndex);

	// RDM Send

	void RdmSendRaw(const uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength);
	void RdmSendDiscoveryRespondMessage(const uint32_t nPortIndex, const uint8_t *pRdmData, uint32_t nLength);

	// RDM Receive

	const uint8_t *RdmReceive(const uint32_t nPortIndex);
	const uint8_t *RdmReceiveTimeOut(const uint32_t nPortIndex, uint16_t nTimeOut);

	// DMX Send

	void SetDmxBreakTime(uint32_t nBreakTime);
	uint32_t GetDmxBreakTime() const;

	void SetDmxMabTime(uint32_t nMabTime);
	uint32_t GetDmxMabTime() const;

	void SetDmxPeriodTime(uint32_t nPeriodTime);
	uint32_t GetDmxPeriodTime() const {
		return m_nDmxTransmitPeriod;
	}

	void SetDmxSlots(uint16_t nSlots = dmx::max::CHANNELS);
	uint16_t GetDmxSlots() const {
		return m_nDmxTransmitSlots;
	}

	void SetSendData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength);
	void SetSendDataWithoutSC(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength);

	void StartOutput(const uint32_t nPortIndex);
	void Sync();

	void SetOutputStyle(const uint32_t nPortIndex, const dmx::OutputStyle outputStyle);
	dmx::OutputStyle GetOutputStyle(const uint32_t nPortIndex) const;

	void Blackout();
	void FullOn();

	// DMX Receive

	const uint8_t *GetDmxAvailable(const uint32_t nPortIndex);
	const uint8_t *GetDmxChanged(const uint32_t nPortIndex);
	const uint8_t *GetDmxCurrentData(const uint32_t nPortIndex);

	uint32_t GetDmxUpdatesPerSecond(const uint32_t nPortIndex);

	static Dmx* Get() {
		return s_pThis;
	}

private:
	void StartData(const uint32_t nPortIndex);
	void StopData(const uint32_t nPortIndex);
	void StartDmxOutput(const uint32_t nPortIndex);

private:
	uint32_t m_nDmxTransmitPeriod { dmx::transmit::PERIOD_DEFAULT };
	uint32_t m_nDmxTransmitPeriodRequested { dmx::transmit::PERIOD_DEFAULT };
	uint32_t m_nDmxTransmissionLength[dmx::config::max::PORTS];
	uint32_t m_nDmxTransmitSlots { dmx::max::CHANNELS };
	dmx::PortDirection m_dmxPortDirection[dmx::config::max::PORTS];
	bool m_bHasContinuosOutput { false };

	static Dmx *s_pThis;
};

#endif /* GD32_DMX_H_ */
