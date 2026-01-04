/**
 * @file rdmdevice.h
 *
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

#ifndef RDMDEVICE_H_
#define RDMDEVICE_H_

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <algorithm>

#include "firmwareversion.h"
#include "hal.h"
#include "rdmdevicestore.h"
#include "rdmidentify.h"
#include "rdmconst.h"
#include "rdm_e120.h"
#include "json/rdmdeviceparams.h"
#if defined(H3)
#include "h3_board.h"
#elif defined(GD32)
#include "gd32_board.h"
#endif
#include "hal_serialnumber.h"
#include "firmware/debug/debug_debug.h"

namespace rdm
{
struct DeviceInfoData
{
    char* data;
    uint8_t length;
};

///< http://rdm.openlighting.org/pid/display?manufacturer=0&pid=96
struct DeviceInfo
{
    uint8_t
        protocol_major; ///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
    uint8_t
        protocol_minor; ///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
    uint8_t device_model[2]; ///< This field identifies the Device Model ID of the Root Device or the Sub-Device. The Manufacturer shall not use the same ID to
                             ///< represent more than one unique model type.
    uint8_t product_category[2]; ///< Devices shall report a Product Category based on the products primary function.
    uint8_t software_version[4]; ///< This field indicates the Software Version ID for the device. The Software Version ID is a 32-bit value determined by the
                                 ///< Manufacturer.
    uint8_t
        dmx_footprint[2]; ///< If the DEVICE_INFO message is directed to a Sub-Device, then the response for this field contains the DMX512 Footprint for that
                          ///< Sub-Device. If the message is sent to the Root Device, it is the Footprint for the Root Device itself. If the Device or
                          ///< Sub-Device does not utilize Null Start Code packets for any control or functionality then it shall report a Footprint of 0x0000.
    uint8_t current_personality;  ///<
    uint8_t personality_count;    ///<
    uint8_t dmx_start_address[2]; ///< If the Device or Sub-Device that this message is directed to has a DMX512 Footprint of 0, then this field shall be set to
                                  ///< 0xFFFF.
    uint8_t sub_device_count[2];  ///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a
                                  ///< Sub-Device.
    uint8_t sensor_count;         ///< This field indicates the number of available sensors in a Root Device or Sub-Device. When this parameter is directed to a
                                  ///< Sub-Device, the reply shall be identical for any Sub-Device owned by a specific Root Device.
};
} // namespace rdm

class RdmDevice
{
#if defined(CONFIG_RDM_DEVICE_ROOT_LABEL)
    static constexpr char kDeviceLabel[] = CONFIG_RDM_DEVICE_ROOT_LABEL;
#else
#if defined(RDM_RESPONDER)
#if defined(H3)
    static constexpr char kDeviceLabel[] = H3_BOARD_NAME " RDM Device";
#elif defined(GD32)
    static constexpr char kDeviceLabel[] = GD32_BOARD_NAME " RDM Device";
#elif defined(RASPPI)
    static constexpr char kDeviceLabel[] = "Raspberry Pi RDM Device";
#elif defined(__linux__)
    static constexpr char kDeviceLabel[] = "Linux RDM Device";
#elif defined(__APPLE__)
    static constexpr char kDeviceLabel[] = "MacOS RDM Device";
#else
    static constexpr char kDeviceLabel[] = "RDM Device";
#endif
#else
    static constexpr char kDeviceLabel[] = "RDMNet LLRP Only Device";
#endif
#endif
   public:
    static RdmDevice& Get()
    {
        static RdmDevice instance; // one instance for the whole program
        return instance;
    }

    void Init()
    {
        DEBUG_ENTRY();

        assert(!s_is_init);

        SetFactoryDefaults();

        s_is_init = true;

        json::RdmDeviceParams rdmdevice_params;
        rdmdevice_params.Load();
        rdmdevice_params.Set();

        DEBUG_EXIT();
    }

    void Print()
    {
        printf("RDM Device configuration [Protocol Version %d.%d]\n", device_info_.protocol_major, device_info_.protocol_minor);
        const auto kLength = static_cast<int>(std::min(static_cast<size_t>(RDM_MANUFACTURER_LABEL_MAX_LENGTH), strlen(RDMConst::MANUFACTURER_NAME)));
        printf(" Manufacturer Name : %.*s\n", kLength, const_cast<char*>(&RDMConst::MANUFACTURER_NAME[0]));
        printf(" Manufacturer ID   : %.2X%.2X\n", s_uid[0], s_uid[1]);
        printf(" Serial Number     : %.2X%.2X%.2X%.2X\n", s_serial_number[3], s_serial_number[2], s_serial_number[1], s_serial_number[0]);
        printf(" Root label        : %.*s\n", s_root_label_length, root_label_);
        printf(" Product Category  : %.2X%.2X\n", product_category_ >> 8, product_category_ & 0xFF);
        printf(" Product Detail    : %.2X%.2X\n", product_detail_ >> 8, product_detail_ & 0xFF);
#if defined(RDM_RESPONDER)
        printf(" DMX Address   : %d\n", (device_info_.dmx_start_address[0] << 8) + device_info_.dmx_start_address[1]);
        printf(" DMX Footprint : %d\n", (device_info_.dmx_footprint[0] << 8) + device_info_.dmx_footprint[1]);
        printf(" Personality %d of %d\n", device_info_.current_personality, device_info_.personality_count);
        printf(" Sub Devices   : %d\n", (device_info_.sub_device_count[0] << 8) + device_info_.sub_device_count[1]);
        printf(" Sensors       : %d\n", device_info_.sensor_count);
#endif
    }

    void SetFactoryDefaults()
    {
        DEBUG_ENTRY();

        const struct rdm::DeviceInfoData kInfoData = {.data = factory_root_label_, .length = s_factory_root_label_length};
        SetLabel(&kInfoData);

        checksum_ = CalculateChecksum();

        DEBUG_EXIT();
    }

    bool GetFactoryDefaults() { return (checksum_ == RdmDevice::CalculateChecksum()); }

    const uint8_t* GetUID() const { return s_uid; }

    const uint8_t* GetSN() const { return s_serial_number; }

    void GetManufacturerId(struct rdm::DeviceInfoData* info_data)
    {
        info_data->data = reinterpret_cast<char*>(const_cast<uint8_t*>(RDMConst::MANUFACTURER_ID));
        info_data->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
    }

    void GetManufacturerName(struct rdm::DeviceInfoData* info_data)
    {
        info_data->data = const_cast<char*>(&RDMConst::MANUFACTURER_NAME[0]);
        info_data->length = static_cast<uint8_t>(std::min(static_cast<size_t>(RDM_MANUFACTURER_LABEL_MAX_LENGTH), strlen(RDMConst::MANUFACTURER_NAME)));
    }

    void SetLabel(const struct rdm::DeviceInfoData* info_data)
    {
        const auto kLength = std::min(static_cast<uint8_t>(RDM_DEVICE_LABEL_MAX_LENGTH), info_data->length);

        if ((kLength > 1) && info_data->data[0] > ' ')
        {
            memcpy(root_label_, info_data->data, kLength);
            s_root_label_length = kLength;

            if (s_is_init)
            {
                rdmdevice_store::SaveLabel(root_label_, s_root_label_length);
            }
        }
    }

    void GetLabel(struct rdm::DeviceInfoData* info_data)
    {
        info_data->data = root_label_;
        info_data->length = s_root_label_length;
    }

    void SetProductCategory(uint16_t product_category) { product_category_ = product_category; }
    uint16_t GetProductCategory() const { return product_category_; }

    void SetProductDetail(uint16_t product_detail) { product_detail_ = product_detail; }
    uint16_t GetProductDetail() const { return product_detail_; }

    rdm::DeviceInfo* GetDeviceInfo() { return &device_info_; }

    // RDM Responder specific

    void SetPersonalityCount(uint8_t count) { device_info_.personality_count = count; }

    uint8_t GetPersonalityCount() const { return device_info_.personality_count; }

    void SetCurrentPersonality(uint8_t current) { device_info_.current_personality = current; }

    uint8_t GetCurrentPersonality() const { return device_info_.current_personality; }

    void SetDmxFootprint(uint16_t dmx_footprint)
    {
        device_info_.dmx_footprint[0] = static_cast<uint8_t>(dmx_footprint >> 8);
        device_info_.dmx_footprint[1] = static_cast<uint8_t>(dmx_footprint);
    }

    uint16_t GetDmxFootprint() const { return static_cast<uint16_t>((device_info_.dmx_footprint[0] << 8) + device_info_.dmx_footprint[1]); }

    void SetDmxStartAddress(uint16_t dmx_start_address)
    {
        device_info_.dmx_start_address[0] = static_cast<uint8_t>(dmx_start_address >> 8);
        device_info_.dmx_start_address[1] = static_cast<uint8_t>(dmx_start_address);
    }

    uint16_t GetDmxStartAddress() const { return static_cast<uint16_t>((device_info_.dmx_start_address[0] << 8) + device_info_.dmx_start_address[1]); }

    void SetSubdeviceCount(uint16_t sub_device_count)
    {
        device_info_.sub_device_count[0] = static_cast<uint8_t>(sub_device_count >> 8);
        device_info_.sub_device_count[1] = static_cast<uint8_t>(sub_device_count);
    }

    uint16_t GetSubdeviceCount() const { return static_cast<uint16_t>((device_info_.sub_device_count[0] << 8) + device_info_.sub_device_count[1]); }

    void SetSensorCount(uint8_t sensor_count) { device_info_.sensor_count = sensor_count; }

    uint8_t GetSensorCount() const { return device_info_.sensor_count; }

   private:
    RdmDevice()
    {
        DEBUG_ENTRY();

        memset(&device_info_, 0, sizeof(struct rdm::DeviceInfo));

        const auto kSoftwareVersionId = FirmwareVersion::Get()->GetVersionId();
        const auto kDeviceModel = hal::kBoardId;
        const auto kProductCategory = E120_PRODUCT_CATEGORY_OTHER;

        device_info_.protocol_major = (E120_PROTOCOL_VERSION >> 8);
        device_info_.protocol_minor = static_cast<uint8_t>(E120_PROTOCOL_VERSION);
        device_info_.device_model[0] = static_cast<uint8_t>(kDeviceModel >> 8);
        device_info_.device_model[1] = static_cast<uint8_t>(kDeviceModel);
        device_info_.product_category[0] = static_cast<uint8_t>(kProductCategory >> 8);
        device_info_.product_category[1] = static_cast<uint8_t>(kProductCategory);
        device_info_.software_version[0] = static_cast<uint8_t>(kSoftwareVersionId >> 24);
        device_info_.software_version[1] = static_cast<uint8_t>(kSoftwareVersionId >> 16);
        device_info_.software_version[2] = static_cast<uint8_t>(kSoftwareVersionId >> 8);
        device_info_.software_version[3] = static_cast<uint8_t>(kSoftwareVersionId);
        device_info_.dmx_start_address[0] = 0xFF;
        device_info_.dmx_start_address[1] = 0xFF;
        device_info_.current_personality = 1;

        hal::SerialNumber(s_serial_number);

        s_uid[0] = RDMConst::MANUFACTURER_ID[0];
        s_uid[1] = RDMConst::MANUFACTURER_ID[1];
        s_uid[2] = s_serial_number[0];
        s_uid[3] = s_serial_number[1];
        s_uid[4] = s_serial_number[2];
        s_uid[5] = s_serial_number[3];

        s_factory_root_label_length = sizeof(kDeviceLabel) - 1;
        memcpy(factory_root_label_, kDeviceLabel, s_factory_root_label_length);
        memcpy(root_label_, kDeviceLabel, s_factory_root_label_length);

        DEBUG_EXIT();
    }

    uint16_t CalculateChecksum()
    {
        uint16_t checksum = s_root_label_length;

        for (uint32_t i = 0; i < s_root_label_length; i++)
        {
            checksum = static_cast<uint16_t>(checksum + root_label_[i]);
        }

        return checksum;
    }

   private:
    RDMIdentify rdm_identify_;
    rdm::DeviceInfo device_info_;
    char factory_root_label_[RDM_DEVICE_LABEL_MAX_LENGTH];
    char root_label_[RDM_DEVICE_LABEL_MAX_LENGTH];

    uint16_t product_category_{E120_PRODUCT_CATEGORY_OTHER};
    uint16_t product_detail_{E120_PRODUCT_DETAIL_OTHER};
    uint16_t checksum_{0};

    uint8_t s_uid[RDM_UID_SIZE];
#define DEVICE_SN_LENGTH 4
    uint8_t s_serial_number[DEVICE_SN_LENGTH];
    uint8_t s_factory_root_label_length{0};
    uint8_t s_root_label_length{0};

    bool s_is_init{false};
};

#endif // RDMDEVICE_H_
