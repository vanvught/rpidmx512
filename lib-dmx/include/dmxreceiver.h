/**
 * @file dmxreceiver.h
 */
/* Copyright (C) 2017-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXRECEIVER_H_
#define DMXRECEIVER_H_

#include <cstdint>
#include <cstdio>

#include "dmx.h"
#include "dmxnode_outputtype.h"
#include "hal_statusled.h"

class DMXReceiver : Dmx
{
   public:
    explicit DMXReceiver(DmxNodeOutputType* dmx_node_output_type) { dmx_node_output_type_ = dmx_node_output_type; }

    ~DMXReceiver()
    {
        DMXReceiver::Stop();
        is_active_ = false;
    }

    void Start() { Dmx::SetPortDirection(0, dmx::PortDirection::kInput, true); }

    void Stop()
    {
        Dmx::SetPortDirection(0, dmx::PortDirection::kInput, false);
        dmx_node_output_type_->Stop(0);
    }

    void SetDmxNodeOutputType(DmxNodeOutputType* dmx_node_output_type)
    {
        if (dmx_node_output_type != dmx_node_output_type_)
        {
            dmx_node_output_type_->Stop(0);
            dmx_node_output_type_ = dmx_node_output_type;
            is_active_ = false;
        }
    }

    const uint8_t* Run(int16_t& length)
    {
        if (__builtin_expect((disable_output_), 0))
        {
            length = 0;
            return nullptr;
        }

        const auto* dmx_available = Dmx::GetDmxAvailable(0);

        if (__builtin_expect((dmx_available != nullptr), 0))
        {
            const auto* dmx_statistics = reinterpret_cast<const struct Data*>(dmx_available);
            length = static_cast<int16_t>(dmx_statistics->Statistics.nSlotsInPacket);

            ++dmx_available;

            dmx_node_output_type_->SetData<true>(0, dmx_available, static_cast<uint16_t>(length));

            if (!is_active_)
            {
                dmx_node_output_type_->Start(0);
                is_active_ = true;
                hal::statusled::SetMode(hal::statusled::Mode::DATA);
            }

            return const_cast<uint8_t*>(dmx_available);
        }
        else if (Dmx::GetDmxUpdatesPerSecond(0) == 0)
        {
            if (is_active_)
            {
                dmx_node_output_type_->Stop(0);
                is_active_ = false;
                hal::statusled::SetMode(hal::statusled::Mode::NORMAL);
            }

            length = -1;
            return nullptr;
        }

        length = 0;
        return nullptr;
    }

    void SetDisableOutput(bool disable = true) { disable_output_ = disable; }

    uint32_t GetUpdatesPerSecond(uint32_t port_index) { return Dmx::GetDmxUpdatesPerSecond(port_index); }

    const uint8_t* GetDmxCurrentData(uint32_t port_index) { return Dmx::GetDmxCurrentData(port_index); }

    void Print() { printf(" Output %s\n", disable_output_ ? "disabled" : "enabled"); }

   private:
    DmxNodeOutputType* dmx_node_output_type_{nullptr};
    bool is_active_{false};
    bool disable_output_{false};
};

#endif  // DMXRECEIVER_H_
