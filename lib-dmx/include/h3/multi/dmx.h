/**
 * @file dmx.h
 *
 */
/* Copyright (C) 2018-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef H3_MULTI_DMX_H_
#define H3_MULTI_DMX_H_

#include <cstdint>

#include "dmxconst.h"
#include "dmx_config.h"
#include "dmxstatistics.h"

struct Statistics
{
    uint32_t nSlotsInPacket;
    uint32_t nSlotToSlot;
    uint32_t nMarkAfterBreak;
    uint32_t nBreakToBreak;
};

struct Data
{
    uint8_t Data[dmx::buffer::kSize];
    struct Statistics Statistics;
};

class Dmx
{
   public:
    Dmx();

    void SetPortDirection(uint32_t port_index, dmx::PortDirection port_direction, bool enable_data = false);
    dmx::PortDirection GetPortDirection(uint32_t port_index) const { return m_dmxPortDirection[port_index]; }

    void ClearData(uint32_t port_index);

    volatile dmx::TotalStatistics& GetTotalStatistics(uint32_t port_index);

    // RDM Send

    void RdmSendRaw(uint32_t port_index, const uint8_t* data, uint32_t length);
    void RdmSendDiscoveryRespondMessage(uint32_t port_index, const uint8_t* data, uint32_t length);

    // RDM Receive

    const uint8_t* RdmReceive(uint32_t port_index);
    const uint8_t* RdmReceiveTimeOut(uint32_t port_index, uint16_t timeout);

    // DMX Send

    void SetDmxBreakTime(uint32_t break_time);
    uint32_t GetDmxBreakTime() const { return m_nDmxTransmitBreakTime; }

    void SetDmxMabTime(uint32_t mab_time);
    uint32_t GetDmxMabTime() const { return m_nDmxTransmitMabTime; }

    void SetDmxPeriodTime(uint32_t period_time);
    uint32_t GetDmxPeriodTime() const { return m_nDmxTransmitPeriod; }

    void SetDmxSlots(uint16_t slots = dmx::kChannelsMax);
    uint16_t GetDmxSlots() const { return m_nDmxTransmitSlots; }

    //	void SetSendData(uint32_t port_index, const uint8_t *pData, uint32_t length, const dmx::SendStyle dmxSendStyle = dmx::SendStyle::kDirect);
    template <dmx::SendStyle dmxSendStyle> void SetSendDataWithoutSC(uint32_t port_index, const uint8_t* data, uint32_t length);

    void Sync();

    void SetOutputStyle(uint32_t port_index, dmx::OutputStyle output_style);
    dmx::OutputStyle GetOutputStyle(uint32_t port_index) const;

    void Blackout();
    void FullOn();

    // DMX Receive

    const uint8_t* GetDmxAvailable(uint32_t port_index);
    const uint8_t* GetDmxChanged(uint32_t port_index);
    const uint8_t* GetDmxCurrentData(uint32_t port_index);

    uint32_t GetDmxUpdatesPerSecond(uint32_t port_index);
    uint32_t GetDmxReceivedCount(uint32_t port_index);

    static Dmx* Get() { return s_this; }

   private:
    void StartData(H3_UART_TypeDef* uart, uint32_t port_index);
    void StopData(H3_UART_TypeDef* uart, uint32_t port_index);

    template <uint32_t portIndex> void SetSendDataInternal(const uint8_t* data, uint32_t length);

    void StartOutput(uint32_t port_index);
    void StartDmxOutput(uint32_t port_index);

   private:
    uint32_t m_nDmxTransmitBreakTime{dmx::transmit::kBreakTimeTypical};
    uint32_t m_nDmxTransmitMabTime{dmx::transmit::kMabTimeMin};
    uint32_t m_nDmxTransmitPeriod{dmx::transmit::kPeriodDefault};
    uint32_t m_nDmxTransmitPeriodRequested{dmx::transmit::kPeriodDefault};
    uint32_t m_nDmxTransmissionLength[dmx::config::max::kPorts];
    uint16_t m_nDmxTransmitSlots{dmx::kChannelsMax};
    dmx::PortDirection m_dmxPortDirection[dmx::config::max::kPorts];

    static Dmx* s_this;
};

#define DMX_HANDLE_SEND_CASE(i) \
    case i:                     \
        return SetSendDataInternal<i>(pData, length)

template <dmx::SendStyle dmxSendStyle> inline void Dmx::SetSendDataWithoutSC(uint32_t port_index, const uint8_t* pData, uint32_t length)
{
    switch (port_index)
    {
        DMX_HANDLE_SEND_CASE(0);
#if DMX_MAX_PORTS >= 2
        DMX_HANDLE_SEND_CASE(1);
#endif
#if DMX_MAX_PORTS >= 3
        DMX_HANDLE_SEND_CASE(2);
#endif
#if DMX_MAX_PORTS == 4
        DMX_HANDLE_SEND_CASE(3);
#endif
        default:
            return;
    }
}

#undef DMX_HANDLE_SEND_CASE

#endif  // H3_MULTI_DMX_H_
