/**
 * @file rdmdeviceresponder.h
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

#ifndef RDMDEVICERESPONDER_H_
#define RDMDEVICERESPONDER_H_

#include <cstdint>
#include <cstring>

#include "firmware/debug/debug_debug.h"
#include "hal.h"
#include "rdmconst.h"
#include "rdmdevice.h"
#include "rdmidentify.h"
#include "rdmpersonality.h"
#include "rdmsensors.h"
#include "rdmsubdevices.h"
#include "dmxnode.h"
#include "dmxnode_outputtype.h"

namespace rdm::device::responder
{
static constexpr uint8_t kDefaultCurrentPersonality = 1;

} // namespace rdm::device::responder
namespace configstore
{
void SetFactoryDefaults();
} // namespace configstore

class RDMDeviceResponder
{
    static constexpr char kLanguage[2] = {'e', 'n'};

   public:
    RDMDeviceResponder(RDMPersonality** personalities, uint32_t personality_count,
                       uint32_t current_personality = rdm::device::responder::kDefaultCurrentPersonality)
        : rdm_personalities_(personalities)
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;

        memset(&sub_device_info_, 0, sizeof(struct rdm::DeviceInfo));

        RdmDevice::Get().SetPersonalityCount(static_cast<uint8_t>(personality_count));
        RdmDevice::Get().SetCurrentPersonality(static_cast<uint8_t>(current_personality));

        assert(current_personality != 0);

        const auto* dmx_node_output_type = rdm_personalities_[current_personality - 1]->GetDmxNodeOutputType();

        if (dmx_node_output_type == nullptr)
        {
            dmx_start_address_factory_default_ = dmxnode::kAddressInvalid;
        }

        DEBUG_EXIT();
    }

    virtual ~RDMDeviceResponder() = default;

    void Init()
    {
        DEBUG_ENTRY();

        auto& rdm_device = RdmDevice::Get();

        rdm_device.Init();

        const auto kSubDevices = rdm_sub_devices_.GetCount();

        const auto kCurrentPersonality = rdm_device.GetCurrentPersonality();

        assert(kCurrentPersonality != 0);
        auto* dmx_node_output_type = rdm_personalities_[kCurrentPersonality - 1]->GetDmxNodeOutputType();
	
        if (dmx_node_output_type == nullptr)
        {
            rdm_device.SetDmxFootprint(0);
            rdm_device.SetDmxStartAddress(dmx_start_address_factory_default_);
        }
        else
        {
            rdm_device.SetDmxFootprint(dmx_node_output_type->GetDmxFootprint());
            rdm_device.SetDmxStartAddress(dmx_node_output_type->GetDmxStartAddress());
        }

        rdm_device.SetSubdeviceCount(kSubDevices);
        rdm_device.SetSensorCount(rdm_sensors_.GetCount());

        memcpy(&sub_device_info_, rdm_device.GetDeviceInfo(), sizeof(struct rdm::DeviceInfo));

        checksum_ = CalculateChecksum();

        DEBUG_EXIT();
    }

    void Print() { RdmDevice::Get().Print(); }

    // E120_DEVICE_INFO				0x0060
    struct rdm::DeviceInfo* GetDeviceInfo(uint16_t sub_device = RDM_ROOT_DEVICE)
    {
        if (sub_device != RDM_ROOT_DEVICE)
        {
            const auto* sub_device_info = rdm_sub_devices_.GetInfo(sub_device);

            if (sub_device_info != nullptr)
            {
                sub_device_info_.dmx_footprint[0] = static_cast<uint8_t>(sub_device_info->dmx_footprint >> 8);
                sub_device_info_.dmx_footprint[1] = static_cast<uint8_t>(sub_device_info->dmx_footprint);
                sub_device_info_.current_personality = sub_device_info->current_personality;
                sub_device_info_.personality_count = sub_device_info->personality_count;
                sub_device_info_.dmx_start_address[0] = static_cast<uint8_t>(sub_device_info->dmx_start_address >> 8);
                sub_device_info_.dmx_start_address[1] = static_cast<uint8_t>(sub_device_info->dmx_start_address);
                sub_device_info_.sensor_count = sub_device_info->sensor_count;
            }

            return &sub_device_info_;
        }

        return RdmDevice::Get().GetDeviceInfo();
    }

    // E120_DEVICE_LABEL			0x0082
    void SetLabel(uint16_t sub_device, const char* label, uint8_t label_length)
    {
        struct rdm::DeviceInfoData info;

        if (label_length > RDM_DEVICE_LABEL_MAX_LENGTH)
        {
            label_length = RDM_DEVICE_LABEL_MAX_LENGTH;
        }

        if (sub_device != RDM_ROOT_DEVICE)
        {
            rdm_sub_devices_.SetLabel(sub_device, label, label_length);
            return;
        }

        info.data = const_cast<char*>(label);
        info.length = label_length;

        RdmDevice::Get().SetLabel(&info);
    }

    void GetLabel(uint16_t sub_device, struct rdm::DeviceInfoData* info)
    {
        if (sub_device != RDM_ROOT_DEVICE)
        {
            rdm_sub_devices_.GetLabel(sub_device, info);
            return;
        }

        RdmDevice::Get().GetLabel(info);
    }

    // E120_FACTORY_DEFAULTS		0x0090
    void SetFactoryDefaults()
    {
        DEBUG_ENTRY();
        auto& rdm_device = RdmDevice::Get();

        rdm_device.SetFactoryDefaults();

        assert(rdm_personalities_ != nullptr);

        SetPersonalityCurrent(RDM_ROOT_DEVICE, rdm::device::responder::kDefaultCurrentPersonality);
        SetDmxStartAddress(RDM_ROOT_DEVICE, dmx_start_address_factory_default_);

        memcpy(&sub_device_info_, rdm_device.GetDeviceInfo(), sizeof(struct rdm::DeviceInfo));

        rdm_sub_devices_.SetFactoryDefaults();

        checksum_ = CalculateChecksum();
        is_factory_defaults_ = true;

        configstore::SetFactoryDefaults();

        DEBUG_EXIT();
    }

    bool GetFactoryDefaults()
    {
        if (is_factory_defaults_)
        {
            if (!RdmDevice::Get().GetFactoryDefaults())
            {
                is_factory_defaults_ = false;
                return false;
            }

            if (checksum_ != CalculateChecksum())
            {
                is_factory_defaults_ = false;
                return false;
            }

            if (!rdm_sub_devices_.GetFactoryDefaults())
            {
                is_factory_defaults_ = false;
                return false;
            }
        }

        return is_factory_defaults_;
    }

    // E120_LANGUAGE				0x00B0
    void SetLanguage(const char language[2])
    {
        language_[0] = language[0];
        language_[1] = language[1];
    }
    const char* GetLanguage() const { return language_; }

    // E120_DMX_START_ADDRESS		0x00F0
    void SetDmxStartAddress(uint16_t sub_device, uint16_t dmx_start_address)
    {
        DEBUG_ENTRY();

        if (dmx_start_address == 0 || dmx_start_address > dmxnode::kUniverseSize) return;

        if (sub_device != RDM_ROOT_DEVICE)
        {
            rdm_sub_devices_.SetDmxStartAddress(sub_device, dmx_start_address);
            return;
        }

        const auto kCurrentPersonality = RdmDevice::Get().GetCurrentPersonality();
        assert(kCurrentPersonality >= 1);
        const auto* personality = rdm_personalities_[kCurrentPersonality - 1];
        assert(personality != nullptr);

        auto* dmx_node_output_type = personality->GetDmxNodeOutputType();

        if (dmx_node_output_type != nullptr)
        {
            if (dmx_node_output_type->SetDmxStartAddress(dmx_start_address))
            {
                RdmDevice::Get().SetDmxStartAddress(dmx_start_address);
            }

            DmxStartAddressUpdate();
        }

        DEBUG_EXIT();
    }

    uint16_t GetDmxStartAddress(uint16_t sub_device = RDM_ROOT_DEVICE)
    {
        if (sub_device != RDM_ROOT_DEVICE)
        {
            return rdm_sub_devices_.GetDmxStartAddress(sub_device);
        }

        return RdmDevice::Get().GetDmxStartAddress();
    }

    // E120_SLOT_INFO				0x0120
    bool GetSlotInfo(uint16_t sub_device, uint16_t slot_offset, dmxnode::SlotInfo& slot_info)
    {
        if (sub_device != RDM_ROOT_DEVICE)
        {
            return false; // TODO(a): GetSlotInfo SubDevice
        }

        const auto kCurrentPersonality = RdmDevice::Get().GetCurrentPersonality();
        assert(kCurrentPersonality >= 1);
        const auto* personality = rdm_personalities_[kCurrentPersonality - 1];
        auto* dmx_node_output_type = personality->GetDmxNodeOutputType();

        return dmx_node_output_type->GetSlotInfo(slot_offset, slot_info);
    }

    uint16_t GetDmxFootPrint(uint16_t sub_device = RDM_ROOT_DEVICE)
    {
        if (sub_device != RDM_ROOT_DEVICE)
        {
            return rdm_sub_devices_.GetDmxFootPrint(sub_device);
        }

        return RdmDevice::Get().GetDmxFootprint();
    }

    // Personalities
    RDMPersonality* GetPersonality(uint16_t sub_device, uint8_t personality)
    {
        assert(personality >= 1);

        if (sub_device != RDM_ROOT_DEVICE)
        {
            return rdm_sub_devices_.GetPersonality(sub_device, personality);
        }

        assert(personality <= RdmDevice::Get().GetPersonalityCount());

        return rdm_personalities_[personality - 1];
    }

    uint8_t GetPersonalityCount(uint16_t sub_device = RDM_ROOT_DEVICE)
    {
        if (sub_device != RDM_ROOT_DEVICE)
        {
            return rdm_sub_devices_.GetPersonalityCount(sub_device);
        }

        return RdmDevice::Get().GetPersonalityCount();
    }

    void SetPersonalityCurrent(uint16_t sub_device, uint8_t personality)
    {
        assert(personality >= 1);

        if (sub_device != RDM_ROOT_DEVICE)
        {
            rdm_sub_devices_.SetPersonalityCurrent(sub_device, personality);
            return;
        }
		
		auto& rdm_device = RdmDevice::Get();

		rdm_device.SetCurrentPersonality(personality);
		
        assert(personality <= rdm_device.GetCurrentPersonality());

        const auto* p_personality = rdm_personalities_[personality - 1];
        assert(p_personality != nullptr);

        auto* dmx_node_output_type = p_personality->GetDmxNodeOutputType();

        if (dmx_node_output_type != nullptr)
        {
			rdm_device.SetDmxFootprint(dmx_node_output_type->GetDmxFootprint());
			rdm_device.SetDmxStartAddress(dmx_node_output_type->GetDmxStartAddress());
		
            PersonalityUpdate(dmx_node_output_type);
        }
    }

    uint8_t GetPersonalityCurrent(uint16_t sub_device = RDM_ROOT_DEVICE)
    {
        if (sub_device != RDM_ROOT_DEVICE)
        {
            return rdm_sub_devices_.GetPersonalityCurrent(sub_device);
        }

        return RdmDevice::Get().GetCurrentPersonality();
    }

    virtual void PersonalityUpdate([[maybe_unused]] DmxNodeOutputType* dmx_node_output_type) {}
    virtual void DmxStartAddressUpdate() {}

    static RDMDeviceResponder* Get() { return s_this; }

   private:
    uint16_t CalculateChecksum()
    {
		auto& rdm_device = RdmDevice::Get();
		
        auto checksum = rdm_device.GetDmxStartAddress();
        checksum = static_cast<uint16_t>(checksum + rdm_device.GetCurrentPersonality());
        return checksum;
    }

   private:
    RDMSensors rdm_sensors_;
    RDMSubDevices rdm_sub_devices_;
    RDMPersonality** rdm_personalities_;
    rdm::DeviceInfo sub_device_info_;
    char language_[2]{kLanguage[0], kLanguage[1]};
    bool is_factory_defaults_{true};
    uint16_t checksum_{0};
    uint16_t dmx_start_address_factory_default_{dmxnode::kStartAddressDefault};

    static inline RDMDeviceResponder* s_this;
};

#endif // RDMDEVICERESPONDER_H_
