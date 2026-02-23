/**
 * @file dmxsend.h
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

#ifndef DMXSEND_H_
#define DMXSEND_H_

#include <cstdint>
#include <climits>
#include <cstdio>
#include <cassert>

#include "dmxnode.h"
#include "dmxnodedata.h"
#include "dmx.h"
#if !defined(CONFIG_DMXSEND_DISABLE_CONFIGUDP)
#include "dmxconfigudp.h"
#endif
#include "hal_panelled.h"
#include "hal.h"
 #include "firmware/debug/debug_debug.h"

class DmxSend
{
   public:
    void Start(uint32_t port_index)
    {
        DEBUG_ENTRY();
        DEBUG_PRINTF("port_index=%d", port_index);

        assert(port_index < CHAR_BIT);

        if (IsStarted(started_, port_index))
        {
            DEBUG_EXIT();
            return;
        }

        started_ = static_cast<uint8_t>(started_ | (1U << port_index));

        Dmx::Get()->SetPortDirection(port_index, dmx::PortDirection::kOutput, true);

        if (Dmx::Get()->GetOutputStyle(port_index) == dmx::OutputStyle::kConstant)
        {
            hal::panelled::On(hal::panelled::PORT_A_TX << port_index);
        }

        DEBUG_EXIT();
    }

    void Stop(uint32_t port_index)
    {
        DEBUG_ENTRY();
        DEBUG_PRINTF("port_index=%d -> %u", port_index, IsStarted(started_, static_cast<uint8_t>(port_index)));

        assert(port_index < CHAR_BIT);

        if (!IsStarted(started_, port_index))
        {
            DEBUG_EXIT();
            return;
        }

        started_ = static_cast<uint8_t>(started_ & ~(1U << port_index));

        Dmx::Get()->SetPortDirection(port_index, dmx::PortDirection::kOutput, false);

        hal::panelled::Off(hal::panelled::PORT_A_TX << port_index);

        DEBUG_EXIT();
    }

    template <bool doUpdate> void SetData(uint32_t port_index, const uint8_t* data, uint32_t length)
    {
        assert(port_index < CHAR_BIT);
        assert(data != nullptr);
        assert(length != 0);

        if constexpr (doUpdate)
        {
            Dmx::Get()->SetSendDataWithoutSC<doUpdate ? dmx::SendStyle::kDirect : dmx::SendStyle::kSync>(port_index, data, length);
            hal::panelled::On(hal::panelled::PORT_A_TX << port_index);
        }
    }

    void Sync(uint32_t port_index)
    {
        const auto kLightsetOffset = port_index + dmxnode::kDmxportOffset;
        assert(dmxnode::Data::GetLength(kLightsetOffset) != 0);
        Dmx::Get()->SetSendDataWithoutSC<dmx::SendStyle::kSync>(port_index, dmxnode::Data::Backup(kLightsetOffset), dmxnode::Data::GetLength(kLightsetOffset));
    }

    void Sync()
    {
        Dmx::Get()->Sync();

        for (uint32_t port_index = 0; port_index < dmx::config::max::PORTS; port_index++)
        {
            const auto kLightsetOffset = port_index + dmxnode::kDmxportOffset;
            if (dmxnode::Data::GetLength(kLightsetOffset) != 0)
            {
                dmxnode::Data::ClearLength(kLightsetOffset);
                hal::panelled::On(hal::panelled::PORT_A_TX << port_index);
                if (!IsStarted(started_, port_index))
                {
                    Start(port_index);
                }
            }
        }
    }

#if defined(OUTPUT_HAVE_STYLESWITCH)
    void SetOutputStyle(uint32_t port_index, dmxnode::OutputStyle output_style) { 
		Dmx::Get()->SetOutputStyle(port_index, output_style == dmxnode::OutputStyle::kConstant ? dmx::OutputStyle::kConstant : dmx::OutputStyle::kDelta);
	}

    dmxnode::OutputStyle GetOutputStyle(uint32_t port_index) const { return Dmx::Get()->GetOutputStyle(port_index) == dmx::OutputStyle::kConstant ? dmxnode::OutputStyle::kConstant : dmxnode::OutputStyle::kDelta; }
#endif

    void Blackout([[maybe_unused]] bool blackout) { Dmx::Get()->Blackout(); }

    void FullOn() { Dmx::Get()->FullOn(); }

    void Print()
    {
        puts("DMX Send");
        printf(" Break time   : %u\n", static_cast<unsigned int>(Dmx::Get()->GetDmxBreakTime()));
        printf(" MAB time     : %u\n", static_cast<unsigned int>(Dmx::Get()->GetDmxMabTime()));
        printf(" Refresh rate : %u\n", static_cast<unsigned int>(1000000U / Dmx::Get()->GetDmxPeriodTime()));
        printf(" Slots        : %u\n", Dmx::Get()->GetDmxSlots());
    }

    /*
     * Art-Net ArtPollReply
     */
    uint32_t GetUserData() { return 0; }
    uint32_t GetRefreshRate() { return 1000000U / Dmx::Get()->GetDmxPeriodTime(); }

    /*
     *
     */

    uint16_t GetDmxStartAddress() { return dmxnode::kStartAddressDefault; }
    bool SetDmxStartAddress([[maybe_unused]] uint16_t dmx_start_address) { return false; }
    uint16_t GetDmxFootprint() { return dmxnode::kUniverseSize; }
    bool GetSlotInfo([[maybe_unused]] uint16_t slot_offset, dmxnode::SlotInfo& slot_info)
    {
        slot_info.type = 0x00;       // ST_PRIMARY
        slot_info.category = 0x0001; // SD_INTENSITY
        return true;
    }

   private:
    static constexpr bool IsStarted(uint8_t v, uint32_t p) { return (v & (1U << p)) == (1U << p); }

   private:
#if !defined(CONFIG_DMXSEND_DISABLE_CONFIGUDP)
    DmxConfigUdp dmx_config_udp_;
#endif
    uint8_t started_{0};
};

#endif  // DMXSEND_H_
