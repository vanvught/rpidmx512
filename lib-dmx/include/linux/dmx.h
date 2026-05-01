/**
 * @file dmx.h
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef LINUX_DMX_H_
#define LINUX_DMX_H_

#include <cstdint>

#include "dmxconst.h"
#include "dmx_config.h"
#include "dmxstatistics.h"

struct Statistics {
    uint32_t nSlotsInPacket;
    uint32_t nSlotToSlot;
    uint32_t nMarkAfterBreak;
    uint32_t nBreakToBreak;
};

struct Data {
    uint8_t Data[dmx::buffer::kSize];
    struct Statistics Statistics;
};

class Dmx {
   public:
    Dmx();

    void SetPortDirection(uint32_t nPortIndex, dmx::PortDirection portDirection, bool bEnableData = false);
    dmx::PortDirection GetPortDirection(const uint32_t nPortIndex) { return m_tDmxPortDirection[nPortIndex]; }

    volatile dmx::TotalStatistics& GetTotalStatistics(const uint32_t nPortIndex);

    // RDM Send

    void RdmSendRaw(uint32_t nPortIndex, const uint8_t* pRdmData, uint32_t nLength);
    void RdmSendDiscoveryRespondMessage(const uint32_t nPortIndex, const uint8_t* pRdmData, uint32_t nLength);

    // RDM Receive

    const uint8_t* RdmReceive(uint32_t nPortIndex);
    const uint8_t* RdmReceiveTimeOut(uint32_t nPortIndex, uint16_t nTimeOut);
    uint32_t RdmGetDateReceivedEnd();

    // DMX Send

    void SetSendDataWithoutSC(const uint32_t nPortIndex, const uint8_t* pData, uint32_t nLength, const dmx::SendStyle dmxSendStyle = dmx::SendStyle::kDirect);

    void StartOutput(uint32_t nPortIndex);
    void SetOutput(const bool doForce);

    void SetOutputStyle([[maybe_unused]] const uint32_t nPortIndex, [[maybe_unused]] const dmx::OutputStyle outputStyle) {}
    dmx::OutputStyle GetOutputStyle([[maybe_unused]] const uint32_t nPortIndex) const { return dmx::OutputStyle::kDelta; }

    void Blackout();
    void FullOn();

    void SetDmxBreakTime(uint32_t break_time);
    uint32_t GetDmxBreakTime() const { return m_nDmxTransmitBreakTime; }

    void SetDmxMabTime(uint32_t mab_time);
    uint32_t GetDmxMabTime() const { return m_nDmxTransmitMabTime; }

    void SetDmxPeriodTime(uint32_t nPeriodTime);
    uint32_t GetDmxPeriodTime() const { return m_nDmxTransmitPeriod; }

    void SetDmxSlots(uint16_t nSlots = dmx::kChannelsMax);
    uint16_t GetDmxSlots() const { return m_nDmxTransmitSlots; }

    // DMX Receive

    const uint8_t* GetDmxAvailable(uint32_t nPortIndex);
    const uint8_t* GetDmxCurrentData(uint32_t nPortIndex);
    const uint8_t* GetDmxChanged(uint32_t nPortIndex);

    uint32_t GetDmxUpdatesPerSecond(uint32_t nPortIndex);
    uint32_t GetDmxReceivedCount(uint32_t nPortIndex);

    static Dmx* Get() { return s_this; }

   private:
    void ClearData(uint32_t nUart);
    void StartData(uint32_t nUart, uint32_t nPortIndex);
    void StopData(uint32_t nUart, uint32_t nPortIndex);

   private:
    uint32_t m_nDmxTransmitBreakTime{dmx::transmit::kBreakTimeMin};
    uint32_t m_nDmxTransmitMabTime{dmx::transmit::kMabTimeMin};
    uint32_t m_nDmxTransmitPeriod{dmx::transmit::kPeriodDefault};
    uint32_t m_nDmxTransmitPeriodRequested{dmx::transmit::kPeriodDefault};
    uint16_t m_nDmxTransmitSlots{dmx::kChannelsMax};
    dmx::PortDirection m_tDmxPortDirection[dmx::config::max::kPorts];
    uint32_t m_nDmxTransmissionLength[dmx::config::max::kPorts];

    static Dmx* s_this;
};

#endif /* LINUX_DMX_H_ */
