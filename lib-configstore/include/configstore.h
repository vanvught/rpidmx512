/**
 * @file configstore.h
 *
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

#ifndef CONFIGSTORE_H_
#define CONFIGSTORE_H_

#include <cstdint>
#include <cstring>
#include <cassert>

#include "configstoredevice.h"
#include "configurationstore.h"
#include "global.h"
#include "softwaretimers.h"
 #include "firmware/debug/debug_debug.h"

class ConfigStore : StoreDevice
{
    static constexpr uint32_t kStoreSize = 4 * 1024;
    static constexpr uint8_t kMagicNumber[configurationstore::kMagicNumberSize] = {'A', 'v', 'V', '\0'};
    static constexpr uint8_t kVersion[configurationstore::kVersionSize] = {0, 1};

    static_assert(sizeof(ConfigurationStore) <= kStoreSize);

    enum class State
    {
        kIdle,
        kChanged,
        kChangedWaiting,
        kErasing,
        kErased,
        kErasedWaiting,
        kWriting
    };

    [[maybe_unused]] static constexpr char kStateNames[7][16] = {"IDLE", "CHANGED", "CHANGED_WAITING", "ERASING", "ERASED", "ERASED_WAITING", "WRITING"};

   public:
    ConfigStore()
    {
        DEBUG_ENTRY();

        assert(s_this == nullptr);
        s_this = this;
        
        memset(s_store, 0, sizeof(s_store));

        s_have_device = StoreDevice::IsDetected();

        DEBUG_PRINTF("s_have_device=%u", s_have_device);

        if (s_have_device)
        {
            assert(kStoreSize <= StoreDevice::GetSize());

            const auto kEraseSize = StoreDevice::GetSectorSize();
            assert(kEraseSize <= kStoreSize);

            const auto kSectors = kStoreSize / kEraseSize;

            DEBUG_PRINTF("kStoreSize=%u, kEraseSize=%u, kSectors=%u", kStoreSize, kEraseSize, kSectors);

            assert((kSectors * kEraseSize) <= StoreDevice::GetSize());

            s_start_address = StoreDevice::GetSize() - (kSectors * kEraseSize);

            DEBUG_PRINTF("s_start_address=%p", reinterpret_cast<void*>(s_start_address));

            storedevice::Result result;
            while (!StoreDevice::Read(s_start_address, kStoreSize, reinterpret_cast<uint8_t*>(&s_store), result));
            assert(result == storedevice::Result::kOk);
        }

        auto* store = GetStore();

        if (!IsValid())
        {
            DEBUG_PUTS("Wrong Magic number or version");

            memset(s_store, 0, sizeof(s_store));
            memcpy(store->magic_number, &kMagicNumber, sizeof(kMagicNumber));
            memcpy(store->version, &kVersion, sizeof(kVersion));

            SetStatusChanged();
        }

        // Set global
        Global::Instance().SetUtcOffsetIfValid(store->global.utc_offset);

        DEBUG_EXIT();
    }

    ConfigStore(const ConfigStore&) = delete;
    ConfigStore& operator=(const ConfigStore&) = delete;
    ConfigStore(ConfigStore&&) = delete;
    ConfigStore& operator=(ConfigStore&&) = delete;

    ~ConfigStore() = default;

    bool Commit() { return Flash(); }

    template <typename TMember> void Copy(TMember* dest, const TMember ConfigurationStore::* member)
    {
        assert(dest != nullptr);
        memcpy(dest, &(GetStore()->*member), sizeof(TMember));
    }

    template <typename TMember> void Store(const TMember* source, TMember ConfigurationStore::* member)
    {
        assert(source != nullptr);

        auto* destination = &(GetStore()->*member);
        
        if (__builtin_memcmp(destination, source, sizeof(TMember)) != 0)
        {
            __builtin_memcpy(destination, source, sizeof(TMember));
            SetStatusChanged();
        }
    }

#define DEFINE_STORE_GETTERS_HELPERS(HumanName, StoreName, StoreType) \
    template <typename TField> TField HumanName##Get(TField common::store::StoreType::* pField) const { return Get(GetStore()->StoreName, pField); }

    DEFINE_STORE_GETTERS_HELPERS(Global, global, Global)
    DEFINE_STORE_GETTERS_HELPERS(Network, network, Network)
    DEFINE_STORE_GETTERS_HELPERS(RemoteConfig, remote_config, RemoteConfig)
    DEFINE_STORE_GETTERS_HELPERS(DisplayUdf, display_udf, DisplayUdf)
    DEFINE_STORE_GETTERS_HELPERS(DmxNode, dmx_node, DmxNode)
    DEFINE_STORE_GETTERS_HELPERS(OscClient, osc_client, OscClient)
    DEFINE_STORE_GETTERS_HELPERS(OscServer, osc_server, OscServer)
    DEFINE_STORE_GETTERS_HELPERS(DmxSend, dmx_send, DmxSend)
    DEFINE_STORE_GETTERS_HELPERS(DmxL6470, dmx_l6470, DmxL6470)
    DEFINE_STORE_GETTERS_HELPERS(DmxLed, dmx_led, DmxLed)
    DEFINE_STORE_GETTERS_HELPERS(DmxPwm, dmx_pwm, DmxPwm)
    DEFINE_STORE_GETTERS_HELPERS(DmxSerial, dmx_serial, DmxSerial)
    DEFINE_STORE_GETTERS_HELPERS(DmxMonitor, dmx_monitor, DmxMonitor)
    DEFINE_STORE_GETTERS_HELPERS(RdmDevice, rdm_device, RdmDevice)
    DEFINE_STORE_GETTERS_HELPERS(RdmSensors, rdm_sensors, RdmSensors)
    DEFINE_STORE_GETTERS_HELPERS(RdmSubdevices, rdm_subdevices, RdmSubdevices)
    DEFINE_STORE_GETTERS_HELPERS(ShowFile, show_file, ShowFile)
    DEFINE_STORE_GETTERS_HELPERS(Ltc, ltc, Ltc)
    DEFINE_STORE_GETTERS_HELPERS(LtcDisplay, ltc_display, LtcDisplay)
    DEFINE_STORE_GETTERS_HELPERS(LtcEtc, ltc_etc, LtcEtc)
    DEFINE_STORE_GETTERS_HELPERS(TCNet, tcnet, TcNet)
    DEFINE_STORE_GETTERS_HELPERS(Gps, gps, Gps)
    DEFINE_STORE_GETTERS_HELPERS(Midi, midi, Midi)
    DEFINE_STORE_GETTERS_HELPERS(RgbPanel, rgb_panel, RgbPanel)
    DEFINE_STORE_GETTERS_HELPERS(Widget, widget, Widget)

#undef DEFINE_STORE_GETTERS_HELPERS

#define DEFINE_STORE_UPDATE_HELPERS(HumanName, StoreName, StoreType) \
    template <typename TField> void HumanName##Update(TField common::store::StoreType::* pField, const TField& value) { Update(GetStore()->StoreName, pField, value); }

    DEFINE_STORE_UPDATE_HELPERS(Global, global, Global)
    DEFINE_STORE_UPDATE_HELPERS(RemoteConfig, remote_config, RemoteConfig)
    DEFINE_STORE_UPDATE_HELPERS(Network, network, Network)
    DEFINE_STORE_UPDATE_HELPERS(DisplayUdf, display_udf, DisplayUdf)
    DEFINE_STORE_UPDATE_HELPERS(DmxNode, dmx_node, DmxNode)
    DEFINE_STORE_UPDATE_HELPERS(OscClient, osc_client, OscClient)
    DEFINE_STORE_UPDATE_HELPERS(OscServer, osc_server, OscServer)
    DEFINE_STORE_UPDATE_HELPERS(DmxSend, dmx_send, DmxSend)
    DEFINE_STORE_UPDATE_HELPERS(DmxL6470, dmx_l6470, DmxL6470)
    DEFINE_STORE_UPDATE_HELPERS(DmxLed, dmx_led, DmxLed)
    DEFINE_STORE_UPDATE_HELPERS(DmxPwm, dmx_pwm, DmxPwm)
    DEFINE_STORE_UPDATE_HELPERS(DmxSerial, dmx_serial, DmxSerial)
    DEFINE_STORE_UPDATE_HELPERS(DmxMonitor, dmx_monitor, DmxMonitor)
    DEFINE_STORE_UPDATE_HELPERS(RdmDevice, rdm_device, RdmDevice)
    DEFINE_STORE_UPDATE_HELPERS(RdmSensors, rdm_sensors, RdmSensors)
    DEFINE_STORE_UPDATE_HELPERS(RdmSubdevices, rdm_subdevices, RdmSubdevices)
    DEFINE_STORE_UPDATE_HELPERS(ShowFile, show_file, ShowFile)
    DEFINE_STORE_UPDATE_HELPERS(Ltc, ltc, Ltc)
    DEFINE_STORE_UPDATE_HELPERS(LtcDisplay, ltc_display, LtcDisplay)
    DEFINE_STORE_UPDATE_HELPERS(LtcEtc, ltc_etc, LtcEtc)
    DEFINE_STORE_UPDATE_HELPERS(TCNet, tcnet, TcNet)
    DEFINE_STORE_UPDATE_HELPERS(Gps, gps, Gps)
    DEFINE_STORE_UPDATE_HELPERS(Midi, midi, Midi)
    DEFINE_STORE_UPDATE_HELPERS(RgbPanel, rgb_panel, RgbPanel)
    DEFINE_STORE_UPDATE_HELPERS(Widget, widget, Widget)

#undef DEFINE_STORE_UPDATE_HELPERS

    void SetFlagRemoteConfig(uint32_t flag) { SetFlagInternal(GetStore()->remote_config, &common::store::RemoteConfig::flags, flag); }
    void SetFlagNetwork(uint32_t flag) { SetFlagInternal(GetStore()->network, &common::store::Network::flags, flag); }
    void SetFlagDisplayUdf(uint32_t flag) { SetFlagInternal(GetStore()->display_udf, &common::store::DisplayUdf::flags, flag); }
    void SetFlagDmxNode(uint32_t flag) { SetFlagInternal(GetStore()->dmx_node, &common::store::DmxNode::flags, flag); }
    void SetFlagOscClient(uint32_t flag) { SetFlagInternal(GetStore()->osc_client, &common::store::OscClient::flags, flag); }
    void SetFlagOscServer(uint32_t flag) { SetFlagInternal(GetStore()->osc_server, &common::store::OscServer::flags, flag); }
    void SetFlagDmxSend(uint32_t flag) { SetFlagInternal(GetStore()->dmx_send, &common::store::DmxSend::flags, flag); }
    void SetFlagDmxLed(uint32_t flag) { SetFlagInternal(GetStore()->dmx_led, &common::store::DmxLed::flags, flag); }
    void SetFlagDmxPwm(uint32_t flag) { SetFlagInternal(GetStore()->dmx_pwm, &common::store::DmxPwm::flags, flag); }
    void SetFlagDmxSerial(uint32_t flag) { SetFlagInternal(GetStore()->dmx_serial, &common::store::DmxSerial::set_list, flag); }
    void SetFlagDmxMonitor(uint32_t flag) { SetFlagInternal(GetStore()->dmx_monitor, &common::store::DmxMonitor::set_list, flag); }
    void SetFlagRdmDevice(uint32_t flag) { SetFlagInternal(GetStore()->rdm_device, &common::store::RdmDevice::set_list, flag); }
    void SetFlagShowFile(uint32_t flag) { SetFlagInternal(GetStore()->show_file, &common::store::ShowFile::flags, flag); }
    void SetFlagLtc(uint32_t flag) { SetFlagInternal(GetStore()->ltc, &common::store::Ltc::flags, flag); }
    void SetFlagLtcDisplay(uint32_t flag) { SetFlagInternal(GetStore()->ltc_display, &common::store::LtcDisplay::flags, flag); }
    void SetFlagLtcEtc(uint32_t flag) { SetFlagInternal(GetStore()->ltc_etc, &common::store::LtcEtc::set_list, flag); }
    void SetFlagTCNet(uint32_t flag) { SetFlagInternal(GetStore()->tcnet, &common::store::TcNet::flags, flag); }
    void SetFlagGps(uint32_t flag) { SetFlagInternal(GetStore()->gps, &common::store::Gps::flags, flag); }
    void SetFlagMidi(uint32_t flag) { SetFlagInternal(GetStore()->midi, &common::store::Midi::flags, flag); }
    void SetFlagRgbPanel(uint32_t flag) { SetFlagInternal(GetStore()->rgb_panel, &common::store::RgbPanel::set_list, flag); }
    void SetFlagWidget(uint32_t flag) { SetFlagInternal(GetStore()->widget, &common::store::Widget::set_list, flag); }

    void ClearFlagRemoteConfig(uint32_t flag) { ClearFlagInternal(GetStore()->remote_config, &common::store::RemoteConfig::flags, flag); }
    void ClearFlagNetwork(uint32_t flag) { ClearFlagInternal(GetStore()->network, &common::store::Network::flags, flag); }
    void ClearFlagDisplayUdf(uint32_t flag) { ClearFlagInternal(GetStore()->display_udf, &common::store::DisplayUdf::flags, flag); }
    void ClearFlagDmxNode(uint32_t flag) { ClearFlagInternal(GetStore()->dmx_node, &common::store::DmxNode::flags, flag); }
    void ClearFlagOscClient(uint32_t flag) { ClearFlagInternal(GetStore()->osc_client, &common::store::OscClient::flags, flag); }
    void ClearFlagOscServer(uint32_t flag) { ClearFlagInternal(GetStore()->osc_server, &common::store::OscServer::flags, flag); }
    void ClearFlagDmxSend(uint32_t flag) { ClearFlagInternal(GetStore()->dmx_send, &common::store::DmxSend::flags, flag); }
    void ClearFlagDmxLed(uint32_t flag) { ClearFlagInternal(GetStore()->dmx_led, &common::store::DmxLed::flags, flag); }
    void ClearFlagDmxPwm(uint32_t flag) { ClearFlagInternal(GetStore()->dmx_pwm, &common::store::DmxPwm::flags, flag); }
    void ClearFlagDmxSerial(uint32_t flag) { ClearFlagInternal(GetStore()->dmx_serial, &common::store::DmxSerial::set_list, flag); }
    void ClearFlagDmxMonitor(uint32_t flag) { ClearFlagInternal(GetStore()->dmx_monitor, &common::store::DmxMonitor::set_list, flag); }
    void ClearFlagRdmDevice(uint32_t flag) { ClearFlagInternal(GetStore()->rdm_device, &common::store::RdmDevice::set_list, flag); }
    void ClearFlagShowFile(uint32_t flag) { ClearFlagInternal(GetStore()->show_file, &common::store::ShowFile::flags, flag); }
    void ClearFlagLtc(uint32_t flag) { ClearFlagInternal(GetStore()->ltc, &common::store::Ltc::flags, flag); }
    void ClearFlagLtcDisplay(uint32_t flag) { ClearFlagInternal(GetStore()->ltc_display, &common::store::LtcDisplay::flags, flag); }
    void ClearFlagLtcEtc(uint32_t flag) { ClearFlagInternal(GetStore()->ltc_etc, &common::store::LtcEtc::set_list, flag); }
    void ClearFlagTCNet(uint32_t flag) { ClearFlagInternal(GetStore()->tcnet, &common::store::TcNet::flags, flag); }
    void ClearFlagGps(uint32_t flag) { ClearFlagInternal(GetStore()->gps, &common::store::Gps::flags, flag); }
    void ClearFlagMidi(uint32_t flag) { ClearFlagInternal(GetStore()->midi, &common::store::Midi::flags, flag); }
    void ClearFlagRgbPanel(uint32_t flag) { ClearFlagInternal(GetStore()->rgb_panel, &common::store::RgbPanel::set_list, flag); }
    void ClearFlagWidget(uint32_t flag) { ClearFlagInternal(GetStore()->widget, &common::store::Widget::set_list, flag); }

    bool IsFlagSetRemoteConfig(uint32_t flag) const { return IsFlagSetInternal(GetStore()->remote_config, &common::store::RemoteConfig::flags, flag); }
    bool IsFlagSetNetwork(uint32_t flag) const { return IsFlagSetInternal(GetStore()->network, &common::store::Network::flags, flag); }
    bool IsFlagSetDisplayUdf(uint32_t flag) const { return IsFlagSetInternal(GetStore()->display_udf, &common::store::DisplayUdf::flags, flag); }
    bool IsFlagSetDmxNode(uint32_t flag) const { return IsFlagSetInternal(GetStore()->dmx_node, &common::store::DmxNode::flags, flag); }
    bool IsFlagSetOscClient(uint32_t flag) const { return IsFlagSetInternal(GetStore()->osc_client, &common::store::OscClient::flags, flag); }
    bool IsFlagSetOscServer(uint32_t flag) const { return IsFlagSetInternal(GetStore()->osc_server, &common::store::OscServer::flags, flag); }
    bool IsFlagSetDmxSend(uint32_t flag) const { return IsFlagSetInternal(GetStore()->dmx_send, &common::store::DmxSend::flags, flag); }
    bool IsFlagSetDmxLed(uint32_t flag) const { return IsFlagSetInternal(GetStore()->dmx_led, &common::store::DmxLed::flags, flag); }
    bool IsFlagSetDmxPwm(uint32_t flag) const { return IsFlagSetInternal(GetStore()->dmx_pwm, &common::store::DmxPwm::flags, flag); }
    bool IsFlagSetDmxSerial(uint32_t flag) const { return IsFlagSetInternal(GetStore()->dmx_serial, &common::store::DmxSerial::set_list, flag); }
    bool IsFlagSetDmxMonitor(uint32_t flag) const { return IsFlagSetInternal(GetStore()->dmx_monitor, &common::store::DmxMonitor::set_list, flag); }
    bool IsFlagSetRdmDevice(uint32_t flag) const { return IsFlagSetInternal(GetStore()->rdm_device, &common::store::RdmDevice::set_list, flag); }
    bool IsFlagSetShowFile(uint32_t flag) const { return IsFlagSetInternal(GetStore()->show_file, &common::store::ShowFile::flags, flag); }
    bool IsFlagSetLtc(uint32_t flag) const { return IsFlagSetInternal(GetStore()->ltc, &common::store::Ltc::flags, flag); }
    bool IsFlagSetLtcDisplay(uint32_t flag) const { return IsFlagSetInternal(GetStore()->ltc_display, &common::store::LtcDisplay::flags, flag); }
    bool IsFlagSetLtcEtc(uint32_t flag) const { return IsFlagSetInternal(GetStore()->ltc_etc, &common::store::LtcEtc::set_list, flag); }
    bool IsFlagSetTCNet(uint32_t flag) const { return IsFlagSetInternal(GetStore()->tcnet, &common::store::TcNet::flags, flag); }
    bool IsFlagSetGps(uint32_t flag) const { return IsFlagSetInternal(GetStore()->gps, &common::store::Gps::flags, flag); }
    bool IsFlagSetMidi(uint32_t flag) const { return IsFlagSetInternal(GetStore()->midi, &common::store::Midi::flags, flag); }
    bool IsFlagSetRgbPanel(uint32_t flag) const { return IsFlagSetInternal(GetStore()->rgb_panel, &common::store::RgbPanel::set_list, flag); }
    bool IsFlagSetWidget(uint32_t flag) const { return IsFlagSetInternal(GetStore()->widget, &common::store::Widget::set_list, flag); }

    template <std::size_t N> void RemoteConfigUpdateArray(uint8_t (common::store::RemoteConfig::*field)[N], const char* src, uint32_t length)
    {
        UpdateArray(GetStore()->remote_config, field, reinterpret_cast<const uint8_t*>(src), length);
    }

    template <std::size_t N> void RemoteConfigCopyArray(uint8_t (&dest)[N], const uint8_t (common::store::RemoteConfig::*field)[N]) const
    {
        static_assert(N == common::store::remoteconfig::kDisplayNameLength, "Size mismatch");
        memcpy(dest, (GetStore()->remote_config.*field), N);
    }

    template <std::size_t N> void NetworkUpdateArray(uint8_t (common::store::Network::*field)[N], const char* src, uint32_t length)
    {
        UpdateArray(GetStore()->network, field, reinterpret_cast<const uint8_t*>(src), length);
    }

    template <std::size_t N> void LtcDisplayCopyArray(char (&dest)[N], const char (common::store::LtcDisplay::*field)[N]) const
    {
        static_assert(N == common::store::ltc::display::kMaxInfoMessage, "Size mismatch");
        memcpy(dest, (GetStore()->ltc_display.*field), N);
    }

    template <std::size_t N> void RdmDeviceUpdateArray(uint8_t (common::store::RdmDevice::*field)[N], const char* src, uint32_t length)
    {
        UpdateArray(GetStore()->rdm_device, field, reinterpret_cast<const uint8_t*>(src), length);
    }
    
    uint8_t RdmSensorsIndexedGetType(uint32_t index) const
    {
        assert(index < common::store::rdm::sensors::kMaxDevices);
        return GetStore()->rdm_sensors.entry[index].type;
    }
    
    uint8_t RdmSensorsIndexedGetAddress(uint32_t index) const
    {
        assert(index < common::store::rdm::sensors::kMaxDevices);
        return GetStore()->rdm_sensors.entry[index].address;
    }

    template <typename T, std::size_t N> void RdmSensorsUpdateIndexed(T (common::store::RdmSensors::*field)[N], uint32_t index, const T& value)
    {
        static_assert(N == common::store::rdm::sensors::kMaxSensors, "Array size mismatch");
        assert(index < N);

        auto& array = GetStore()->rdm_sensors.*field;

        if (array[index] != value)
        {
            array[index] = value;
            SetStatusChanged();
        }
    }

    template <std::size_t N> void DmxNodeUpdateArray(uint8_t (common::store::DmxNode::*field)[N], const char* src, uint32_t length)
    {
        UpdateArray(GetStore()->dmx_node, field, reinterpret_cast<const uint8_t*>(src), length);
    }

    template <std::size_t N>
    void DmxNodeUpdateLabel(uint8_t (common::store::DmxNode::*field)[common::store::dmxnode::kParamPorts][N], uint32_t index, const char* src, uint32_t length)
    {
        static_assert(N == common::store::dmxnode::kLabelNameLength, "Label size mismatch");
        assert(index < common::store::dmxnode::kParamPorts);
        assert(src != nullptr);

        auto& labels = GetStore()->dmx_node.*field;

        if (length > N)
        {
            length = N;
        }

        if (__builtin_memcmp(labels[index], src, length) != 0)
        {
            memset(labels[index], 0, N);
            memcpy(labels[index], src, length);
            SetStatusChanged();
        }
    }

    template <typename T, std::size_t N> void DmxNodeUpdateIndexed(T (common::store::DmxNode::*field)[N], uint32_t index, const T& value)
    {
        static_assert(N == common::store::dmxnode::kParamPorts, "Array size mismatch");
        assert(index < N);

        auto& array = GetStore()->dmx_node.*field;

        if (array[index] != value)
        {
            array[index] = value;
            SetStatusChanged();
        }
    }

    uint16_t DmxLedIndexedGetStartUniverse(uint32_t index) const
    {
        assert(index < 16);
        return GetStore()->dmx_led.start_universe[index];
    }

    void DmxL6470CopySparkFunGlobal(common::store::l6470dmx::SparkFun* dest) const
    {
        assert(dest != nullptr);
        *dest = GetStore()->dmx_l6470.spark_fun_global;
    }

    void DmxL6470CopySparkFunIndexed(uint32_t index, common::store::l6470dmx::SparkFun* dest) const
    {
        assert(index < common::store::l6470dmx::kMaxMotors);
        assert(dest != nullptr);
        *dest = GetStore()->dmx_l6470.store[index].spark_fun;
    }

    void DmxL6470CopyModeIndexed(uint32_t index, common::store::l6470dmx::Mode* dest) const
    {
        assert(index < common::store::l6470dmx::kMaxMotors);
        assert(dest != nullptr);
        *dest = GetStore()->dmx_l6470.store[index].mode;
    }

    void DmxL6470CopyL6470Indexed(uint32_t index, common::store::l6470dmx::L6470* dest) const
    {
        assert(index < common::store::l6470dmx::kMaxMotors);
        assert(dest != nullptr);
        *dest = GetStore()->dmx_l6470.store[index].l6470;
    }

    void DmxL6470CopyMotorIndexed(uint32_t index, common::store::l6470dmx::Motor* dest) const
    {
        assert(index < common::store::l6470dmx::kMaxMotors);
        assert(dest != nullptr);
        *dest = GetStore()->dmx_l6470.store[index].motor;
    }

    void DmxL6470StoreSparkFunGlobal(const common::store::l6470dmx::SparkFun* src)
    {
        assert(src != nullptr);
        auto& dest = GetStore()->dmx_l6470.spark_fun_global;

        if (__builtin_memcmp(&dest, src, sizeof(common::store::l6470dmx::SparkFun)) != 0)
        {
            __builtin_memcpy(&dest, src, sizeof(common::store::l6470dmx::SparkFun));
            SetStatusChanged();
        }
    }

    void DmxL6470StoreSparkFunIndexed(uint32_t index, const common::store::l6470dmx::SparkFun* src)
    {
        assert(index < common::store::l6470dmx::kMaxMotors);
        assert(src != nullptr);
        auto& ref = GetStore()->dmx_l6470.store[index].spark_fun;
        if (__builtin_memcmp(&ref, src, sizeof(common::store::l6470dmx::SparkFun)) != 0)
        {
            __builtin_memcpy(&ref, src, sizeof(common::store::l6470dmx::SparkFun));
            SetStatusChanged();
        }
    }

    void DmxL6470StoreModeIndexed(uint32_t index, const common::store::l6470dmx::Mode* src)
    {
        assert(index < common::store::l6470dmx::kMaxMotors);
        assert(src != nullptr);

        auto& ref = GetStore()->dmx_l6470.store[index].mode;
        if (__builtin_memcmp(&ref, src, sizeof(common::store::l6470dmx::Mode)) != 0)
        {
            __builtin_memcpy(&ref, src, sizeof(common::store::l6470dmx::Mode));
            SetStatusChanged();
        }
    }

    void DmxL6470StoreL6470Indexed(uint32_t index, const common::store::l6470dmx::L6470* src)
    {
        assert(index < common::store::l6470dmx::kMaxMotors);
        assert(src != nullptr);
        auto& ref = GetStore()->dmx_l6470.store[index].l6470;
        if (__builtin_memcmp(&ref, src, sizeof(common::store::l6470dmx::L6470)) != 0)
        {
            __builtin_memcpy(&ref, src, sizeof(common::store::l6470dmx::L6470));
            SetStatusChanged();
        }
    }

    void DmxL6470StoreMotorIndexed(uint32_t index, const common::store::l6470dmx::Motor* src)
    {
        assert(index < common::store::l6470dmx::kMaxMotors);
        assert(src != nullptr);
        auto& ref = GetStore()->dmx_l6470.store[index].motor;
        if (__builtin_memcmp(&ref, src, sizeof(common::store::l6470dmx::Motor)) != 0)
        {
            __builtin_memcpy(&ref, src, sizeof(common::store::l6470dmx::Motor));
            SetStatusChanged();
        }
    }

    template <typename TField> 
    TField DmxL6470GetModeIndexed(uint32_t index, TField common::store::l6470dmx::Mode::* field) const
    {
        assert(index < common::store::l6470dmx::kMaxMotors);
        return (GetStore()->dmx_l6470.store[index].mode).*field;
    }
    
    template <typename TField> 
    TField DmxL6470GetMotorIndexed(uint32_t index, TField common::store::l6470dmx::Motor::* field) const
    {
        assert(index < common::store::l6470dmx::kMaxMotors);
        return (GetStore()->dmx_l6470.store[index].motor).*field;
    }

    static ConfigStore& Instance()
    {
        assert(s_this != nullptr);
        return *s_this;
    }

   private:
    template <typename TObject, typename TField> TField Get(const TObject& object, TField TObject::* field) const { return object.*field; }

    template <typename TObject, typename TField> void Update(TObject& object, TField TObject::* field, const TField& value)
    {
        assert(field != nullptr);

        auto* dest = &(object.*field);

        if (__builtin_memcmp(dest, &value, sizeof(TField)) != 0)
        {
            __builtin_memcpy(dest, &value, sizeof(TField));
            SetStatusChanged();
        }
    }

    template <typename TObject, typename TArray, std::size_t N>
    void UpdateArray(TObject& object, TArray (TObject::*field)[N], const TArray* src, uint32_t length)
    {
        assert(src != nullptr);

        auto* dest = &(object.*field);

        if (length > N)
        {
            length = N;
        }

        if (__builtin_memcmp(dest, src, length * sizeof(TArray)) != 0)
        {
            memset(dest, 0, sizeof(TArray) * N);
            memcpy(dest, src, length * sizeof(TArray));
            SetStatusChanged();
        }
    }

   private:
    void SetStatusChanged()
    {
        s_state = State::kChanged;
        TimerStart();
    }

    static void Timer([[maybe_unused]] TimerHandle_t timer_handle)
    {
        DEBUG_ENTRY();

        if (!Instance().Commit())
        {
            Instance().TimerStop();

            DEBUG_EXIT();
            return;
        }

        DEBUG_EXIT();
    }

    void TimerStart()
    {
        DEBUG_ENTRY();
       	DEBUG_PRINTF("s_timer_id=%d", s_timer_id);

        if (s_timer_id != kTimerIdNone)
        {
			DEBUG_EXIT();
            return;
        }

        s_timer_id = SoftwareTimerAdd(100, Timer);

		DEBUG_PRINTF("s_timer_id=%d", s_timer_id);
        DEBUG_EXIT();
    }

    void TimerStop()
    {
        DEBUG_ENTRY();
		DEBUG_PRINTF("s_timer_id=%d", s_timer_id);

        if (s_timer_id == kTimerIdNone)
        {
            return;
            DEBUG_EXIT();
        }

        SoftwareTimerDelete(s_timer_id);

		DEBUG_PRINTF("s_timer_id=%d", s_timer_id);
        DEBUG_EXIT();
    }

    bool Flash()
    {
        DEBUG_PUTS(kStateNames[static_cast<unsigned int>(s_state)]);

        if (__builtin_expect((s_state == State::kIdle), 1))
        {
            return false;
        }

        switch (s_state)
        {
            case State::kChanged:
                s_state = State::kChangedWaiting;
                return true;
            case State::kChangedWaiting:
                s_state = State::kErasing;
                return true;
                break;
            case State::kErasing:
            {
                storedevice::Result result;
                if (StoreDevice::Erase(s_start_address, kStoreSize, result))
                {
                    s_state = State::kErasedWaiting;
                }
                assert(result == storedevice::Result::kOk);
                return true;
            }
            break;
            case State::kErasedWaiting:
                s_state = State::kErased;
                return true;
                break;
            case State::kErased:
                s_state = State::kWriting;
                SoftwareTimerChange(s_timer_id, 0);
                return true;
                break;
            case State::kWriting:
            {  
                storedevice::Result result;
                if (StoreDevice::Write(s_start_address, sizeof(ConfigurationStore), reinterpret_cast<uint8_t*>(&s_store), result))
                {
                    s_state = State::kIdle;
                    return false;
                }
                assert(result == storedevice::Result::kOk);
                return true;
            }
            break;
            default:
                assert(0);
                __builtin_unreachable();
                break;
        }

        assert(0);
        __builtin_unreachable();
        return false;
    }

    ConfigurationStore* GetStore() { return reinterpret_cast<ConfigurationStore*>(s_store); }
    const ConfigurationStore* GetStore() const { return reinterpret_cast<const ConfigurationStore*>(s_store); }

    template <typename TObject> void SetFlagInternal(TObject& object, uint32_t TObject::* field, uint32_t flag)
    {
        auto& flags = object.*field;
        if ((flags & flag) == 0)
        {
            flags |= flag;
            SetStatusChanged();
        }
    }

    template <typename TObject> void ClearFlagInternal(TObject& object, uint32_t TObject::* field, uint32_t flag)
    {
        auto& flags = object.*field;
        if ((flags & flag) != 0)
        {
            flags &= ~flag;
            SetStatusChanged();
        }
    }

    template <typename TObject> bool IsFlagSetInternal(const TObject& object, uint32_t TObject::* field, uint32_t flag) const
    {
        const auto& flags = object.*field;
        return (flags & flag) != 0;
    }

    bool IsValid()
    {
        const auto* store = GetStore();
        return memcmp(store->magic_number, kMagicNumber, sizeof(kMagicNumber)) == 0 && memcmp(store->version, kVersion, sizeof(kVersion)) == 0;
    }

   private:
    static inline uint8_t s_store[kStoreSize];
    static inline uint32_t s_start_address{0};
    static inline bool s_have_device{false};
    static inline State s_state{State::kIdle};
    static inline TimerHandle_t s_timer_id = kTimerIdNone;
    static inline ConfigStore* s_this;
};

inline void ConfigstoreCommit()
{
    while (ConfigStore::Instance().Commit())
    {
    }
}

#endif  // CONFIGSTORE_H_
