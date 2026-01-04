/**
 * @file dmxnodechain.h
 */
/* Copyright (C) 2025 by Arjan van Vught mailto:info@gd32-dmx.org
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
 

#ifndef DMXNODECHAIN_H_
#define DMXNODECHAIN_H_

#include <cstdint>
#include <algorithm>
#include <cassert>

#include "dmxnode.h"
#include "sparkfundmx.h"
#include "tlc59711dmx.h"

 #include "firmware/debug/debug_debug.h"

class DmxNodeChain
{
   public:
    DmxNodeChain() = default;
    ~DmxNodeChain() = default;

    void SetSparkfunDmx(SparkFunDmx* spark_fun_dmx)
    {
        DEBUG_ENTRY();
        assert(spark_fun_dmx != nullptr);
        spark_fun_dmx_ = spark_fun_dmx;

        Set<SparkFunDmx>(spark_fun_dmx);
        DEBUG_EXIT();
    }

    void SetTLC59711Dmx(TLC59711Dmx* tlc59711_dmx)
    {
        DEBUG_ENTRY();
        tlc59711_dmx_ = tlc59711_dmx;

        if (tlc59711_dmx_ == nullptr)
        {
            DEBUG_EXIT();
            return;
        }

        Set<TLC59711Dmx>(tlc59711_dmx);
        DEBUG_EXIT();
    }

    void Start(uint32_t port_index)
    {
        assert(spark_fun_dmx_ != nullptr);
        spark_fun_dmx_->Start(port_index);

        if (tlc59711_dmx_ != nullptr)
        {
            tlc59711_dmx_->Start(port_index);
        }
    }

    void Stop(uint32_t port_index)
    {
        assert(spark_fun_dmx_ != nullptr);
        spark_fun_dmx_->Stop(port_index);

        if (tlc59711_dmx_ != nullptr)
        {
            tlc59711_dmx_->Stop(port_index);
        }
    }

    template <bool doUpdate> void SetData(uint32_t port_index, const uint8_t* data, uint32_t length)
    {
        assert(data != nullptr);
        assert(spark_fun_dmx_ != nullptr);
        spark_fun_dmx_->SetData<doUpdate>(port_index, data, length);

        if (tlc59711_dmx_ != nullptr)
        {
            tlc59711_dmx_->SetData<doUpdate>(port_index, data, length);
        }
    }

    void Sync(uint32_t port_index)
    {
        assert(spark_fun_dmx_ != nullptr);
        spark_fun_dmx_->Sync(port_index);

        if (tlc59711_dmx_ != nullptr)
        {
            tlc59711_dmx_->Sync(port_index);
        }
    }

    void Sync()
    {
        assert(spark_fun_dmx_ != nullptr);
        spark_fun_dmx_->Sync();

        if (tlc59711_dmx_ != nullptr)
        {
            tlc59711_dmx_->Sync();
        }
    }

#if defined(OUTPUT_HAVE_STYLESWITCH)
    void SetOutputStyle([[maybe_unused]] uint32_t port_index, [[maybe_unused]] dmxnode::OutputStyle output_style)
    {
		DEBUG_ENTRY(); 
		DEBUG_EXIT();
    }

    dmxnode::OutputStyle GetOutputStyle([[maybe_unused]] uint32_t port_index) const
    {
        return dmxnode::OutputStyle::kDelta;
    }
#endif

    uint16_t GetDmxFootprint() { return dmx_footprint_; }

    bool SetDmxStartAddress(uint16_t dmx_start_address)
    {
        DEBUG_ENTRY();

        if (dmx_start_address == dmx_start_address_)
        {
            DEBUG_EXIT();
            return true;
        }

        const auto kCurrentDmxStartAddressSparkfun = spark_fun_dmx_->GetDmxStartAddress();
        const auto kNewDmxStartAddressSparkfun = static_cast<uint16_t>((kCurrentDmxStartAddressSparkfun - dmx_start_address_) + dmx_start_address);
        spark_fun_dmx_->SetDmxStartAddress(kNewDmxStartAddressSparkfun);

        if (tlc59711_dmx_ != nullptr)
        {
            const auto kCurrentDmxStartAddress = tlc59711_dmx_->GetDmxStartAddress();
            const auto kNewDmxStartAddress = static_cast<uint16_t>((kCurrentDmxStartAddress - dmx_start_address_) + dmx_start_address);
            tlc59711_dmx_->SetDmxStartAddress(kNewDmxStartAddress);
        }

        dmx_start_address_ = dmx_start_address;

        DEBUG_EXIT();
        return true;
    }

    uint16_t GetDmxStartAddress() { return dmx_start_address_; }

    bool GetSlotInfo(uint16_t slot_offset, dmxnode::SlotInfo& slot_info)
    {
        DEBUG_ENTRY();

        if (slot_offset > dmx_footprint_)
        {
            DEBUG_EXIT();
            return false;
        }

        auto b = GetSlotInfo<SparkFunDmx>(spark_fun_dmx_, slot_offset, slot_info);

        if (b)
        {
            DEBUG_EXIT();
            return true;
        }

        b = (tlc59711_dmx_ != nullptr) && GetSlotInfo<TLC59711Dmx>(tlc59711_dmx_, slot_offset, slot_info);

        DEBUG_EXIT();
        return b;
    }

    uint32_t GetUserData() { return 0; }    ///< Art-Net ArtPollReply
    uint32_t GetRefreshRate() { return 0; } ///< Art-Net ArtPollReply

    void Blackout([[maybe_unused]] bool blackout)
    {
        DEBUG_ENTRY();
        DEBUG_EXIT();
    }

    void FullOn()
    {
        DEBUG_ENTRY();
        DEBUG_EXIT();
    }

    void Print()
    {
        spark_fun_dmx_->Print();
        if (tlc59711_dmx_ != nullptr)
        {
            tlc59711_dmx_->Print();
        }
    }

   private:
    template <class T> void Set(T* t)
    {
		DEBUG_ENTRY();
        assert(t != nullptr);
        
        DEBUG_PRINTF("t->GetDmxStartAddress()=%u,t->GetDmxFootprint()=%u", t->GetDmxStartAddress(),t->GetDmxFootprint());

        if (dmx_start_address_ == dmxnode::kAddressInvalid)
        {
            dmx_start_address_ = t->GetDmxStartAddress();
            dmx_footprint_ = t->GetDmxFootprint();

            DEBUG_PRINTF("dmx_start_address_=%d, dmx_footprint_=%d", dmx_start_address_, dmx_footprint_);
            DEBUG_EXIT();
            return;
        }

        DEBUG_PRINTF("t->GetDmxStartAddress()=%d, t->GetDmxFootprint()=%d\n", t->GetDmxStartAddress(), t->GetDmxFootprint());

        const auto kDmxChannelLastCurrent = static_cast<uint16_t>(dmx_start_address_ + dmx_footprint_);
        dmx_start_address_ = std::min(dmx_start_address_, t->GetDmxStartAddress());

        const auto kDmxChannelLast = static_cast<uint16_t>(t->GetDmxStartAddress() + t->GetDmxFootprint());
        dmx_footprint_ = static_cast<uint16_t>(std::max(kDmxChannelLastCurrent, kDmxChannelLast) - dmx_start_address_);

        DEBUG_PRINTF("dmx_start_address_=%d, dmx_footprint_=%d\n", dmx_start_address_, dmx_footprint_);
        DEBUG_EXIT();
    }

    template <class T> bool GetSlotInfo(T* t, uint16_t slot_offset, dmxnode::SlotInfo& slot_info)
    {
        assert(t != nullptr);

        const auto kDmxAddress = dmx_start_address_ + slot_offset;
        const auto kOffset = static_cast<int32_t>(kDmxAddress - t->GetDmxStartAddress());

#ifndef NDEBUG
        printf("\tkOffset=%d, dmx_start_address_=%d, t->GetDmxStartAddress()=%d, t->GetDmxFootprint()=%d\n", static_cast<int>(kOffset), static_cast<int>(dmx_start_address_), static_cast<int>(t->GetDmxStartAddress()),
               static_cast<int>(t->GetDmxFootprint()));

        printf("\nkOffset=%d\n", kOffset);
#endif

        if ((t->GetDmxStartAddress() + t->GetDmxFootprint() <= kDmxAddress) || (kOffset < 0))
        {
            DEBUG_EXIT();
            return false;
        }

        const auto kB = t->GetSlotInfo(static_cast<uint16_t>(kOffset), slot_info);
        DEBUG_EXIT();
        return kB;
    }

   private:
    SparkFunDmx* spark_fun_dmx_{nullptr};
    TLC59711Dmx* tlc59711_dmx_{nullptr};
    uint16_t dmx_start_address_{dmxnode::kAddressInvalid};
    uint16_t dmx_footprint_{0};
};

#endif  // DMXNODECHAIN_H_
