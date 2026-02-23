/**
 * @file rdmsubdevices.h
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

#ifndef RDMSUBDEVICES_H_
#define RDMSUBDEVICES_H_

#include <cstdint>
#include <cassert>

#include "rdmsubdevice.h"

#ifndef NDEBUG
#include "subdevice/rdmsubdevicedummy.h"
#endif

#include "rdmpersonality.h"

 #include "firmware/debug/debug_debug.h"

#if defined(NODE_RDMNET_LLRP_ONLY)
#undef CONFIG_RDM_ENABLE_SUBDEVICES
#endif

namespace rdm::subdevices
{
static constexpr auto MAX = 8;
static constexpr auto STORE = 96; ///< Configuration store in bytes
} // namespace rdm::subdevices

class RDMSubDevices
{
   public:
    RDMSubDevices()
    {
        DEBUG_ENTRY();
        assert(s_this == nullptr);
        s_this = this;

#if defined(CONFIG_RDM_ENABLE_SUBDEVICES)
        rdm_sub_device_ = new RDMSubDevice*[rdm::subdevices::MAX];
        assert(rdm_sub_device_ != nullptr);

#ifndef NDEBUG
        Add(new RDMSubDeviceDummy);
#endif
#endif
        DEBUG_EXIT();
    }

    ~RDMSubDevices()
    {
        DEBUG_ENTRY();
        for (unsigned i = 0; i < count_; i++)
        {
            delete rdm_sub_device_[i];
            rdm_sub_device_[i] = nullptr;
        }

        delete[] rdm_sub_device_;

        count_ = 0;
        DEBUG_EXIT();
    }

    bool Add([[maybe_unused]] RDMSubDevice* rdm_sub_device)
    {
        DEBUG_ENTRY();
#if defined(CONFIG_RDM_ENABLE_SUBDEVICES)
        assert(rdm_sub_device_ != nullptr);

        if (rdm_sub_device_ == nullptr)
        {
            return false;
        }

        if (count_ == rdm::subdevices::MAX)
        {
            DEBUG_EXIT();
            return false;
        }

        assert(rdm_sub_device != nullptr);
        rdm_sub_device_[count_++] = rdm_sub_device;
#endif
        DEBUG_EXIT();
        return true;
    }

    uint16_t GetCount() const { return count_; }

    struct TRDMSubDevicesInfo* GetInfo(uint16_t sub_device)
    {
        assert((sub_device != 0) && (sub_device <= count_));
        assert(rdm_sub_device_[sub_device - 1] != nullptr);
        return rdm_sub_device_[sub_device - 1]->GetInfo();
    }

    uint16_t GetDmxFootPrint(uint16_t sub_device)
    {
        assert((sub_device != 0) && (sub_device <= count_));
        assert(rdm_sub_device_[sub_device - 1] != nullptr);
        return rdm_sub_device_[sub_device - 1]->GetDmxFootPrint();
    }

    RDMPersonality* GetPersonality(uint16_t sub_device, uint8_t personality)
    {
        assert((sub_device != 0) && (sub_device <= count_));
        assert(rdm_sub_device_[sub_device - 1] != nullptr);
        return rdm_sub_device_[sub_device - 1]->GetPersonality(personality);
    }

    uint8_t GetPersonalityCount(uint16_t sub_device)
    {
        assert((sub_device != 0) && (sub_device <= count_));
        assert(rdm_sub_device_[sub_device - 1] != nullptr);
        return rdm_sub_device_[sub_device - 1]->GetPersonalityCount();
    }

    uint8_t GetPersonalityCurrent(uint16_t sub_device)
    {
        assert((sub_device != 0) && (sub_device <= count_));
        assert(rdm_sub_device_[sub_device - 1] != nullptr);
        return rdm_sub_device_[sub_device - 1]->GetPersonalityCurrent();
    }

    void SetPersonalityCurrent(uint16_t sub_device, uint8_t personality)
    {
        assert((sub_device != 0) && (sub_device <= count_));
        assert(rdm_sub_device_[sub_device - 1] != nullptr);
        rdm_sub_device_[sub_device - 1]->SetPersonalityCurrent(personality);
    }

    // E120_DEVICE_LABEL			0x0082
    void GetLabel(uint16_t sub_device, struct rdm::DeviceInfoData* info_data)
    {
        assert((sub_device != 0) && (sub_device <= count_));
        assert(info_data != nullptr);

        assert(rdm_sub_device_[sub_device - 1] != nullptr);
        rdm_sub_device_[sub_device - 1]->GetLabel(info_data);
    }

    void SetLabel(uint16_t sub_device, const char* label, uint8_t label_length)
    {
        assert((sub_device != 0) && (sub_device <= count_));
        assert(label != nullptr);

        assert(rdm_sub_device_[sub_device - 1] != nullptr);
        rdm_sub_device_[sub_device - 1]->SetLabel(label, label_length);
    }

    // E120_FACTORY_DEFAULTS		0x0090
    bool GetFactoryDefaults()
    {
        for (uint32_t i = 0; i < count_; i++)
        {
            if (rdm_sub_device_[i] != nullptr)
            {
                if (!rdm_sub_device_[i]->GetFactoryDefaults())
                {
                    return false;
                }
            }
        }

        return true;
    }

    void SetFactoryDefaults()
    {
        for (uint32_t i = 0; i < count_; i++)
        {
            if (rdm_sub_device_[i] != nullptr)
            {
                rdm_sub_device_[i]->SetFactoryDefaults();
            }
        }
    }

    // E120_DMX_START_ADDRESS		0x00F0
    uint16_t GetDmxStartAddress(uint16_t sub_device)
    {
        assert((sub_device != 0) && (sub_device <= count_));
        assert(rdm_sub_device_[sub_device - 1] != nullptr);
        return rdm_sub_device_[sub_device - 1]->GetDmxStartAddress();
    }

    void SetDmxStartAddress(uint16_t sub_device, uint16_t dmx_start_address)
    {
        assert((sub_device != 0) && (sub_device <= count_));
        assert(rdm_sub_device_[sub_device - 1] != nullptr);
        rdm_sub_device_[sub_device - 1]->SetDmxStartAddress(dmx_start_address);
    }

    void Start()
    {
        DEBUG_ENTRY();
        for (uint32_t i = 0; i < count_; i++)
        {
            if (rdm_sub_device_[i] != nullptr)
            {
                rdm_sub_device_[i]->Start();
            }
        }
        DEBUG_EXIT();
    }

    void Stop()
    {
        DEBUG_ENTRY();
        for (uint32_t i = 0; i < count_; i++)
        {
            if (rdm_sub_device_[i] != nullptr)
            {
                rdm_sub_device_[i]->Stop();
            }
        }
        DEBUG_ENTRY();
    }

    void SetData(const uint8_t* data, uint32_t length)
    {
        for (uint32_t i = 0; i < count_; i++)
        {
            if (rdm_sub_device_[i] != nullptr)
            {
                if (length >= (static_cast<uint16_t>(rdm_sub_device_[i]->GetDmxStartAddress() + rdm_sub_device_[i]->GetDmxFootPrint()) - 1U))
                {
                    rdm_sub_device_[i]->Data(data, length);
                }
            }
        }
    }

    static RDMSubDevices* Get() { return s_this; }

   private:
    RDMSubDevice** rdm_sub_device_{nullptr};
    uint16_t count_{0};

    static inline RDMSubDevices* s_this;
};

#endif  // RDMSUBDEVICES_H_
