/**
 * @file dmx.h
 *
 */
/* Copyright (C) 2015-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef H3_SINGLE_DMX_H_
#define H3_SINGLE_DMX_H_

#include <cstdint>

#include "dmxconst.h"
#include "dmx_config.h"
#include "dmxstatistics.h"

struct Statistics {
    uint32_t slots_in_packet;
    uint32_t slot_to_slot;
    uint32_t mark_after_break;
    uint32_t break_to_break;
};

struct Data {
    uint8_t data[dmx::buffer::kSize];
    struct Statistics statistics;
};

class Dmx {
   public:
    Dmx();

    void SetPortDirection(uint32_t port_index, dmx::PortDirection port_direction, bool enable_data = false);
    dmx::PortDirection GetPortDirection(uint32_t port_index);

    void ClearData(uint32_t port_index);

    volatile dmx::TotalStatistics& GetTotalStatistics(uint32_t port_index);

    // RDM Send
    void RdmSend(uint32_t port_index, const uint8_t* data, uint32_t length);
    void RdmSendDiscoveryRespondMessage(uint32_t port_index, const uint8_t* data, uint32_t length);

    // RDM Receive
    const uint8_t* RdmReceive(uint32_t port_index);
    const uint8_t* RdmReceiveTimeOut(uint32_t port_index, uint16_t time_out);

    // DMX Send
    void SetDmxBreakTime(uint32_t break_time);
    uint32_t GetDmxBreakTime() const { return transmit_break_time_; }

    void SetDmxMabTime(uint32_t mab_time);
    uint32_t GetDmxMabTime() const { return transmit_mab_time_; }

    void SetDmxPeriodTime(uint32_t period_time);
    uint32_t GetDmxPeriodTime() const { return transmit_period_; }

    void SetDmxSlots(uint16_t slots = dmx::kChannelsMax);
    uint16_t GetDmxSlots() const { return transmit_slots_; }

    template <dmx::SendStyle dmxSendStyle> void SetSendData(uint32_t port_index, const uint8_t* data, uint32_t length);

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
    void RdmSendRaw(uint32_t port_index, const uint8_t* data, uint32_t length);

    void StartData(uint32_t port_index);
    void StopData(uint32_t port_index);

    void StartOutput(uint32_t port_index);
    void StartDmxOutput(uint32_t port_index);

   private:
    uint32_t transmit_break_time_{dmx::transmit::kBreakTimeTypical};
    uint32_t transmit_mab_time_{dmx::transmit::kMabTimeMin};
    uint32_t transmit_period_{dmx::transmit::kPeriodDefault};
    uint32_t transmit_period_requested_{dmx::transmit::kPeriodDefault};
    uint32_t transmit_length_[dmx::config::max::kPorts];
    uint16_t transmit_slots_{dmx::kChannelsMax};
    dmx::PortDirection port_direction_[dmx::config::max::kPorts];

    static Dmx* s_this;
};

#endif // H3_SINGLE_DMX_H_
