/**
 * @file remoteconfig.cpp
 *
 */
/* Copyright (C) 2019-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_REMOTECONFIG)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "remoteconfig.h"
#include "firmwareversion.h"
#include "hal.h"
#include "network.h"
#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#include "apps/mdns.h"
#include "dmxnode_nodetype.h"
#include "json/remoteconfigparams.h"
#endif
#include "display.h"
#include "configstore.h"
#include "firmware/debug/debug_dump.h"
 #include "firmware/debug/debug_debug.h"

namespace remoteconfig::udp
{
static constexpr auto kPort = 0x2905;
namespace get
{
enum class Command
{
    kReboot,
    kList,
    kVersion,
    kDisplay,
#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
    kUptime,
#endif
    kTftp,
    kFactory
};
} // namespace get
namespace set
{
enum class Command
{
    kTftp,
    kDisplay
};
} // namespace set
} // namespace remoteconfig::udp

constexpr struct RemoteConfig::Commands RemoteConfig::kGet[] = {
    {&RemoteConfig::HandleReboot, "reboot##", 8, false},     //
    {&RemoteConfig::HandleList, "list#", 5, false},          //
    {&RemoteConfig::HandleVersion, "version#", 8, false},    //
    {&RemoteConfig::HandleDisplayGet, "display#", 8, false}, //
#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
    {&RemoteConfig::HandleUptime, "uptime#", 7, false}, //
#endif
    {&RemoteConfig::HandleTftpGet, "tftp#", 5, false},    //
    {&RemoteConfig::HandleFactory, "factory##", 9, false} //
};

constexpr struct RemoteConfig::Commands RemoteConfig::kSet[] = {
    {&RemoteConfig::HandleTftpSet, "tftp#", 5, true},      //
    {&RemoteConfig::HandleDisplaySet, "display#", 8, true} //
};

static constexpr char kOutput[static_cast<uint32_t>(remoteconfig::Output::LAST)][12] = {"DMX",     "RDM",    "Monitor", "Pixel",  "TimeCode",  "OSC", "Config",
                                                                                        "Stepper", "Player", "Art-Net", "Serial", "RGB Panel", "PWM"};

RemoteConfig::RemoteConfig(remoteconfig::Output output, uint32_t active_outputs)
    : output_(output), active_outputs_(active_outputs)
{
    DEBUG_ENTRY();

    assert(output < remoteconfig::Output::LAST);

    assert(s_this == nullptr);
    s_this = this;

     network::iface::CopyMacAddressTo(s_list.mac_address);
    s_list.output = static_cast<uint8_t>(output);
    s_list.active_outputs = static_cast<uint8_t>(active_outputs);

    handle_ = network::udp::Begin(remoteconfig::udp::kPort, RemoteConfig::StaticCallbackFunction);
    assert(handle_ != -1);

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
    network::apps::mdns::ServiceRecordAdd(nullptr, network::apps::mdns::Services::kConfig);

#if defined(ENABLE_TFTP_SERVER)
    network::apps::mdns::ServiceRecordAdd(nullptr, network::apps::mdns::Services::kTftp);
#endif

#if defined(ENABLE_HTTPD)
    http_daemon_ = new HttpDaemon;
    assert(http_daemon_ != nullptr);
#endif
#endif

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
    json::RemoteConfigParams params;
    params.Load();
    params.Set();
#endif
    DEBUG_EXIT();
}

RemoteConfig::~RemoteConfig()
{
    DEBUG_ENTRY();

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
#if defined(ENABLE_HTTPD)
    if (http_daemon_ != nullptr)
    {
        delete http_daemon_;
    }
#endif

    network::apps::mdns::ServiceRecordDelete(network::apps::mdns::Services::kConfig);
#endif

    network::udp::End(remoteconfig::udp::kPort);
    handle_ = -1;

    DEBUG_EXIT();
}

void RemoteConfig::SetDisplayName(const char* display_name)
{
    DEBUG_ENTRY();
    
    char array[common::store::remoteconfig::kDisplayNameLength];

	size_t len = strlen(display_name);
    len = len > (common::store::remoteconfig::kDisplayNameLength - 1) ? common::store::remoteconfig::kDisplayNameLength - 1 : len;
    memcpy(reinterpret_cast<char*>(array), display_name, len);

    for (uint32_t i = len; i < common::store::remoteconfig::kDisplayNameLength; i++)
    {
        array[i] = '\0';
    }
    
     ConfigStore::Instance().RemoteConfigUpdateArray(&common::store::RemoteConfig::display_name, array, common::store::remoteconfig::kDisplayNameLength);

    DEBUG_EXIT();
}

namespace configstore
{
void SetFactoryDefaults();
}

void RemoteConfig::HandleFactory()
{
    configstore::SetFactoryDefaults();
}

void RemoteConfig::Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    udp_buffer_ = const_cast<char*>(reinterpret_cast<const char*>(buffer));
    bytes_received_ = size;
    ip_from_ = from_ip;
#ifndef NDEBUG
    debug::Dump(udp_buffer_, bytes_received_);
#endif

    if (udp_buffer_[bytes_received_ - 1] == '\n')
    {
        bytes_received_--;
    }

    const Commands* handler = nullptr;

    if (udp_buffer_[0] == '?')
    {
        bytes_received_--;
        for (uint32_t i = 0; i < (sizeof(kGet) / sizeof(kGet[0])); i++)
        {
            if ((kGet[i].kGreaterThan) && (bytes_received_ <= kGet[i].kLength))
            {
                continue;
            }
            if ((!kGet[i].kGreaterThan) && (bytes_received_ != kGet[i].kLength))
            {
                continue;
            }
            if (memcmp(&udp_buffer_[1], kGet[i].cmd, kGet[i].kLength) == 0)
            {
                handler = &kGet[i];
                break;
            }
        }

        if (handler != nullptr)
        {
            (this->*(handler->handler))();
            return;
        }

        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>("ERROR#?\n"), 8, ip_from_, remoteconfig::udp::kPort);
        return;
    }

    if (udp_buffer_[0] == '!')
    {
        bytes_received_--;
        for (uint32_t i = 0; i < (sizeof(kSet) / sizeof(kSet[0])); i++)
        {
            if ((kSet[i].kGreaterThan) && (bytes_received_ <= kSet[i].kLength))
            {
                continue;
            }
            if ((!kSet[i].kGreaterThan) && ((bytes_received_ - 1U) != kSet[i].kLength))
            {
                continue;
            }
            if (memcmp(&udp_buffer_[1], kSet[i].cmd, kSet[i].kLength) == 0)
            {
                handler = &kSet[i];
                break;
            }
        }

        if (handler != nullptr)
        {
            (this->*(handler->handler))();
            return;
        }

        network::udp::Send(handle_, reinterpret_cast<const uint8_t*>("ERROR#!\n"), 8, ip_from_, remoteconfig::udp::kPort);
        return;
    }
}

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
void RemoteConfig::HandleUptime()
{
    DEBUG_ENTRY();

    const auto kUptime = hal::Uptime();
    const auto kLength = snprintf(udp_buffer_, remoteconfig::udp::kBufferSize - 1, "uptime: %us\n", static_cast<unsigned int>(kUptime));

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(udp_buffer_), static_cast<uint32_t>(kLength), ip_from_, remoteconfig::udp::kPort);

    DEBUG_EXIT();
}
#endif

void RemoteConfig::HandleVersion()
{
    DEBUG_ENTRY();

    const auto* p = FirmwareVersion::Get()->GetPrint();
    const auto kLength = snprintf(udp_buffer_, remoteconfig::udp::kBufferSize - 1, "version:%s\n", p);
    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(udp_buffer_), static_cast<uint32_t>(kLength), ip_from_, remoteconfig::udp::kPort);

    DEBUG_EXIT();
}

void RemoteConfig::HandleList()
{
    DEBUG_ENTRY();

    constexpr auto kCmdLength = kGet[static_cast<uint32_t>(remoteconfig::udp::get::Command::kList)].kLength;
    auto* list_response = &udp_buffer_[kCmdLength + 2U];
    const auto kListResponseBufferLength = remoteconfig::udp::kBufferSize - (kCmdLength + 2U);
    int32_t list_length;

    uint8_t display_name[common::store::remoteconfig::kDisplayNameLength];
    ConfigStore::Instance().RemoteConfigCopyArray(display_name, &common::store::RemoteConfig::display_name);
    display_name[common::store::remoteconfig::kDisplayNameLength - 1] = '\0'; // Just to be safe

#if !defined(CONFIG_REMOTECONFIG_MINIMUM)
    const auto* const kNodeTypeName = dmxnode::GetNodeType(dmxnode::kNodeType);
#else
	const auto* const kNodeTypeName = "Bootloader TFTP";
#endif

    if (display_name[0] != '\0')
    {
        list_length = snprintf(list_response, kListResponseBufferLength - 1, "" IPSTR ",%s,%s,%u,%s\n", IP2STR(network::GetPrimaryIp()), kNodeTypeName,
                               kOutput[static_cast<uint32_t>(output_)], static_cast<unsigned int>(active_outputs_), display_name);
    }
    else
    {
        list_length = snprintf(list_response, kListResponseBufferLength - 1, "" IPSTR ",%s,%s,%u\n", IP2STR(network::GetPrimaryIp()), kNodeTypeName,
                               kOutput[static_cast<uint32_t>(output_)], static_cast<unsigned int>(active_outputs_));
    }

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(list_response), static_cast<uint32_t>(list_length), ip_from_, remoteconfig::udp::kPort);

    DEBUG_EXIT();
}

void RemoteConfig::HandleDisplaySet()
{
    DEBUG_ENTRY();

    constexpr auto kCmdLength = kSet[static_cast<uint32_t>(remoteconfig::udp::set::Command::kDisplay)].kLength;

    if (bytes_received_ != (kCmdLength + 1U))
    {
        DEBUG_EXIT();
        return;
    }

    Display::Get()->SetSleep(udp_buffer_[kCmdLength + 1U] == '0');

    DEBUG_PRINTF("%c", udp_buffer_[kCmdLength + 1]);
    DEBUG_EXIT();
}

void RemoteConfig::HandleDisplayGet()
{
    DEBUG_ENTRY();

    const bool kIsOn = !(Display::Get()->IsSleep());
    const auto kLength = snprintf(udp_buffer_, remoteconfig::udp::kBufferSize - 1, "display:%s\n", kIsOn ? "On" : "Off");

    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(udp_buffer_), static_cast<uint32_t>(kLength), ip_from_, remoteconfig::udp::kPort);

    DEBUG_EXIT();
}

void RemoteConfig::TftpExit()
{
    DEBUG_ENTRY();

    const auto kCmdLength = kSet[static_cast<uint32_t>(remoteconfig::udp::set::Command::kTftp)].kLength;

    bytes_received_ = kCmdLength + 1U;
    udp_buffer_[kCmdLength + 1] = '0';

    HandleTftpSet();

    DEBUG_EXIT();
}

void RemoteConfig::HandleTftpSet()
{
    DEBUG_ENTRY();

    constexpr auto kCmdLength = kSet[static_cast<uint32_t>(remoteconfig::udp::set::Command::kTftp)].kLength;

    if (bytes_received_ != (kCmdLength + 1U))
    {
        DEBUG_EXIT();
        return;
    }

    enable_tftp_ = (udp_buffer_[kCmdLength + 1U] != '0');

    if (enable_tftp_)
    {
        Display::Get()->SetSleep(false);
    }

    PlatformHandleTftpSet();

    DEBUG_EXIT();
}

void RemoteConfig::HandleTftpGet()
{
    DEBUG_ENTRY();

    PlatformHandleTftpGet();

    const auto kLength = snprintf(udp_buffer_, remoteconfig::udp::kBufferSize - 1, "tftp:%s\n", enable_tftp_ ? "On" : "Off");
    network::udp::Send(handle_, reinterpret_cast<const uint8_t*>(udp_buffer_), static_cast<uint32_t>(kLength), ip_from_, remoteconfig::udp::kPort);

    DEBUG_EXIT();
}

void RemoteConfig::HandleReboot()
{
    DEBUG_ENTRY();

    is_reboot_ = true;

    Display::Get()->SetSleep(false);
    Display::Get()->Cls();
    Display::Get()->TextStatus("Rebooting ...");

    hal::Reboot();
    __builtin_unreachable();
}
