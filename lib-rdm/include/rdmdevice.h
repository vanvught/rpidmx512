/**
 * @file rdmdevice.h
 *
 */
/* Copyright (C) 2017-2026 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef RDMDEVICE_H_
#define RDMDEVICE_H_

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>

#include "rdm_device_info.h"
#include "rdm_device_base.h"
#include "rdmdevicestore.h"
#include "rdmidentify.h"
#include "rdmconst.h"
#include "rdm_e120.h"
#include "firmware/debug/debug_debug.h"

namespace rdm::device
{
uint16_t DeviceModel();
uint32_t BootSoftwareVersionId();
uint32_t SoftwareVersionId();
const char* SoftwareVersionLabel(uint32_t& length);
const char* RootLabel(uint8_t& length);
void SetFactoryDefaults();

class Device
{
    static constexpr auto kProductCategory =
#if defined(RDM_DEVICE_PRODUCT_CATEGORY)
        RDM_DEVICE_PRODUCT_CATEGORY;
#else
        E120_PRODUCT_CATEGORY_DATA_DISTRIBUTION;
#endif
    static constexpr auto kProductDetail =
#if defined(RDM_DEVICE_PRODUCT_DETAIL)
        RDM_DEVICE_PRODUCT_DETAIL;
#else
        E120_PRODUCT_DETAIL_ETHERNET_NODE;
#endif

   public:
    static Device& Instance()
    {
        static Device instance;
        return instance;
    }

    void Print()
    {
        printf("RDM Device configuration [Protocol Version %d.%d]\n", info_.protocol_major, info_.protocol_minor);
        rdm::device::Base::Instance().Print();
        printf(" Root label        : %.*s\n", root_label_length_, root_label_);
        printf(" Product Category  : %.2X%.2X\n", info_.product_category[0], info_.product_category[1]);
        printf(" Product Detail    : %.4X\n", kProductDetail);
#if defined(RDM_RESPONDER)
        puts("RDM Device Responder");
        printf(" DMX Address   : %d\n", (info_.dmx_start_address[0] << 8) + info_.dmx_start_address[1]);
        printf(" DMX Footprint : %d\n", (info_.dmx_footprint[0] << 8) + info_.dmx_footprint[1]);
        printf(" Personality %d of %d\n", info_.current_personality, info_.personality_count);
        printf(" Sub Devices   : %d\n", (info_.sub_device_count[0] << 8) + info_.sub_device_count[1]);
        printf(" Sensors       : %d\n", info_.sensor_count);
#endif
    }

    void SetFactoryDefaults()
    {
        DEBUG_ENTRY();

        memset(root_label_, 0, sizeof(root_label_));
        rdm::device::store::SaveLabel(root_label_, root_label_length_);

        const auto* const kRootLabel = rdm::device::RootLabel(root_label_length_);
        memcpy(root_label_, kRootLabel, root_label_length_);

        checksum_ = CalculateChecksum();

        DEBUG_EXIT();
    }

    bool GetFactoryDefaults() { return (checksum_ == rdm::device::Device::CalculateChecksum()); }

    void GetManufacturerId(struct rdm::device::InfoData* info_data)
    {
        info_data->data = reinterpret_cast<char*>(const_cast<uint8_t*>(RDMConst::MANUFACTURER_ID));
        info_data->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
    }

    void GetManufacturerName(struct rdm::device::InfoData* info_data)
    {
        info_data->data = const_cast<char*>(&RDMConst::MANUFACTURER_NAME[0]);
        info_data->length = static_cast<uint8_t>(std::min(static_cast<size_t>(RDM_MANUFACTURER_LABEL_MAX_LENGTH), strlen(RDMConst::MANUFACTURER_NAME)));
    }

    void SetLabel(const struct rdm::device::InfoData* info_data)
    {
        const auto kLength = std::min(static_cast<uint8_t>(RDM_DEVICE_LABEL_MAX_LENGTH), info_data->length);

        if ((kLength > 1) && info_data->data[0] > ' ')
        {
            memcpy(root_label_, info_data->data, kLength);
            root_label_length_ = kLength;

            rdm::device::store::SaveLabel(root_label_, root_label_length_);
        }
    }

    void GetLabel(struct rdm::device::InfoData* info_data)
    {
        info_data->data = reinterpret_cast<char*>(root_label_);
        info_data->length = root_label_length_;
    }

    uint16_t GetProductCategory() const { return kProductCategory; }

    uint16_t GetProductDetail() const { return kProductDetail; }

    rdm::device::Info* GetDeviceInfo() { return &info_; }

    // RDM_RESPONDER
    void SetPersonalityCount(uint8_t count) { info_.personality_count = count; }

    uint8_t GetPersonalityCount() const { return info_.personality_count; }

    void SetCurrentPersonality(uint8_t current) { info_.current_personality = current; }

    uint8_t GetCurrentPersonality() const { return info_.current_personality; }

    void SetDmxFootprint(uint16_t dmx_footprint)
    {
        info_.dmx_footprint[0] = static_cast<uint8_t>(dmx_footprint >> 8);
        info_.dmx_footprint[1] = static_cast<uint8_t>(dmx_footprint);
    }

    uint16_t GetDmxFootprint() const { return static_cast<uint16_t>((info_.dmx_footprint[0] << 8) + info_.dmx_footprint[1]); }

    void SetDmxStartAddress(uint16_t dmx_start_address)
    {
        info_.dmx_start_address[0] = static_cast<uint8_t>(dmx_start_address >> 8);
        info_.dmx_start_address[1] = static_cast<uint8_t>(dmx_start_address);
    }

    uint16_t GetDmxStartAddress() const { return static_cast<uint16_t>((info_.dmx_start_address[0] << 8) + info_.dmx_start_address[1]); }

    void SetSubdeviceCount(uint16_t sub_device_count)
    {
        info_.sub_device_count[0] = static_cast<uint8_t>(sub_device_count >> 8);
        info_.sub_device_count[1] = static_cast<uint8_t>(sub_device_count);
    }

    uint16_t GetSubdeviceCount() const { return static_cast<uint16_t>((info_.sub_device_count[0] << 8) + info_.sub_device_count[1]); }

    void SetSensorCount(uint8_t sensor_count) { info_.sensor_count = sensor_count; }

    uint8_t GetSensorCount() const { return info_.sensor_count; }

   private:
    Device()
    {
        DEBUG_ENTRY();

        memset(&info_, 0, sizeof(struct rdm::device::Info));

        const auto kSoftwareVersionId = rdm::device::SoftwareVersionId();
        const auto kDeviceModel = rdm::device::DeviceModel();

        info_.protocol_major = (E120_PROTOCOL_VERSION >> 8);
        info_.protocol_minor = static_cast<uint8_t>(E120_PROTOCOL_VERSION);
        info_.device_model[0] = static_cast<uint8_t>(kDeviceModel >> 8);
        info_.device_model[1] = static_cast<uint8_t>(kDeviceModel);
        info_.product_category[0] = static_cast<uint8_t>(kProductCategory >> 8);
        info_.product_category[1] = static_cast<uint8_t>(kProductCategory);
        info_.software_version[0] = static_cast<uint8_t>(kSoftwareVersionId >> 24);
        info_.software_version[1] = static_cast<uint8_t>(kSoftwareVersionId >> 16);
        info_.software_version[2] = static_cast<uint8_t>(kSoftwareVersionId >> 8);
        info_.software_version[3] = static_cast<uint8_t>(kSoftwareVersionId);
        info_.dmx_start_address[0] = 0xFF;
        info_.dmx_start_address[1] = 0xFF;
        info_.current_personality = 1;

        rdm::device::store::LoadLabel(root_label_, root_label_length_);

        if (root_label_[0] == '\0')
        {
            const auto* const kRootLabel = rdm::device::RootLabel(root_label_length_);
            memcpy(root_label_, kRootLabel, root_label_length_);

            checksum_ = CalculateChecksum();
        }

        DEBUG_EXIT();
    }

    uint16_t CalculateChecksum()
    {
        uint16_t checksum = root_label_length_;

        for (uint32_t i = 0; i < root_label_length_; i++)
        {
            checksum = static_cast<uint16_t>(checksum + root_label_[i]);
        }

        return checksum;
    }

   private:
    RDMIdentify identify_;
    rdm::device::Info info_;
    uint8_t root_label_[RDM_DEVICE_LABEL_MAX_LENGTH];
    uint16_t checksum_{0};
    uint8_t root_label_length_{0};
};
} // namespace rdm::device

#endif // RDMDEVICE_H_
