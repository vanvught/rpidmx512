/**
 * @file dmxnodewith4.h
 */
/* Copyright (C) 2025-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef DMXNODEWITH4_H_
#define DMXNODEWITH4_H_

#include <cstdint>

#include "dmxnode.h"
#include "dmxnode_outputtype.h"
#include "dmxsend.h"
#include "firmware/debug/debug_debug.h"

template <uint32_t nMaxPorts> class DmxNodeWith4
{
   public:
    DmxNodeWith4(DmxPixelOutputType* dmx_pixel_output_type, DmxSend* dmx_send) : dmx_pixel_output_type_(dmx_pixel_output_type), dmx_send_(dmx_send)
    {
        DEBUG_PRINTF("nMaxPorts=%u DmxPixelOutputType=%p DmxSend=%p", nMaxPorts, reinterpret_cast<void*>(dmx_pixel_output_type_),
                     reinterpret_cast<void*>(dmx_send_));

        assert(s_this == nullptr);
        s_this = this;
    }

    ~DmxNodeWith4() = default;

    void SetDmxPixel(DmxPixelOutputType* const kDmxPixelOutputType) { dmx_pixel_output_type_ = kDmxPixelOutputType; }

    DmxPixelOutputType* GetDmxPixel() const { return dmx_pixel_output_type_; }

    void SetDmxSend(DmxSend* const kDmxSend) { dmx_send_ = kDmxSend; }

    DmxSend* GetDmxSend() const { return dmx_send_; }

    void Start(uint32_t port_index)
    {
        if (port_index < nMaxPorts)
        {
            if (dmx_pixel_output_type_ != nullptr)
            {
                return dmx_pixel_output_type_->Start(port_index);
            }
            return;
        }
        if (dmx_send_ != nullptr)
        {
            dmx_send_->Start(port_index & 0x3);
        }
    }

    void Stop(uint32_t port_index)
    {
        if (port_index < nMaxPorts)
        {
            if (dmx_pixel_output_type_ != nullptr)
            {
                return dmx_pixel_output_type_->Stop(port_index);
            }
            return;
        }
        if (dmx_send_ != nullptr)
        {
            dmx_send_->Stop(port_index & 0x3);
        }
    }

    template <bool doUpdate> void SetData(uint32_t port_index, const uint8_t* data, uint32_t length)
    {
        if (port_index < nMaxPorts)
        {
            if (dmx_pixel_output_type_ != nullptr)
            {
                return dmx_pixel_output_type_->SetData<doUpdate>(port_index, data, length);
            }
            return;
        }
        if (dmx_send_ != nullptr)
        {
            return dmx_send_->SetData<doUpdate>(port_index & 0x3, data, length);
        }
    }

    void Sync(uint32_t port_index)
    {
        if (port_index < nMaxPorts)
        {
            if (dmx_pixel_output_type_ != nullptr)
            {
                return dmx_pixel_output_type_->Sync(port_index);
            }
            return;
        }
        if (dmx_send_ != nullptr)
        {
            return dmx_send_->Sync(port_index & 0x3);
        }
    }

    void Sync()
    {
        if (dmx_pixel_output_type_ != nullptr)
        {
            dmx_pixel_output_type_->Sync();
        }
        if (dmx_send_ != nullptr)
        {
            dmx_send_->Sync();
        }
    }

#if defined(OUTPUT_HAVE_STYLESWITCH)
    void SetOutputStyle(uint32_t port_index, const dmxnode::OutputStyle outputStyle)
    {
        if (port_index < nMaxPorts)
        {
            if (dmx_pixel_output_type_ != nullptr)
            {
                return dmx_pixel_output_type_->SetOutputStyle(port_index, outputStyle);
            }
            return;
        }
        if (dmx_send_ != nullptr)
        {
            return dmx_send_->SetOutputStyle(port_index & 0x3, outputStyle);
        }
    }

    dmxnode::OutputStyle GetOutputStyle(uint32_t port_index) const
    {
        if (port_index < nMaxPorts)
        {
            if (dmx_pixel_output_type_ != nullptr)
            {
                return dmx_pixel_output_type_->GetOutputStyle(port_index);
            }
            return dmxnode::OutputStyle::kDelta;
        }
        if (dmx_send_ != nullptr)
        {
            return dmx_send_->GetOutputStyle(port_index & 0x3);
        }

        return dmxnode::OutputStyle::kDelta;
    }
#endif

    void Blackout(bool blackout)
    {
        if (dmx_pixel_output_type_ != nullptr)
        {
            dmx_pixel_output_type_->Blackout(blackout);
        }
        if (dmx_send_ != nullptr)
        {
            dmx_send_->Blackout(blackout);
        }
    }

    void FullOn()
    {
        if (dmx_pixel_output_type_ != nullptr)
        {
            dmx_pixel_output_type_->FullOn();
        }
        if (dmx_send_ != nullptr)
        {
            dmx_send_->FullOn();
        }
    }

    void Print()
    {
        if (dmx_pixel_output_type_ != nullptr)
        {
            dmx_pixel_output_type_->Print();
        }
        if (dmx_send_ != nullptr)
        {
            dmx_send_->Print();
        }
    }

    uint32_t GetUserData()
    {
        if (dmx_pixel_output_type_ != nullptr)
        {
            return dmx_pixel_output_type_->GetUserData();
        }
        return 0;
    }

    uint32_t GetRefreshRate()
    {
        if (dmx_pixel_output_type_ != nullptr)
        {
            return dmx_pixel_output_type_->GetRefreshRate();
        }
        return 0;
    }

    bool SetDmxStartAddress([[maybe_unused]] uint16_t dmx_start_address) { return false; }

    uint16_t GetDmxStartAddress() { return dmxnode::kAddressInvalid; }

    uint16_t GetDmxFootprint() { return 0; }

    bool GetSlotInfo([[maybe_unused]] uint16_t slot_offset, [[maybe_unused]] dmxnode::SlotInfo& slot_info) { return false; }

    static DmxNodeWith4& Get()
    {
        assert(s_this != nullptr); // Ensure that s_this is valid
        return *s_this;
    }

   private:
    DmxPixelOutputType* dmx_pixel_output_type_;
    DmxSend* dmx_send_;

    static inline DmxNodeWith4* s_this;
};

#endif // DMXNODEWITH4_H_
