/**
 * @file oscserver.cpp
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

#if defined(DEBUG_OSCSERVER)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cassert>

#include "oscserver.h"
#include "osc.h"
#include "oscsimplemessage.h"
#include "oscsimplesend.h"
#include "oscblob.h"
#include "dmxnode.h"
#include "hal_boardinfo.h"
#include "firmware/debug/debug_dump.h"
 #include "firmware/debug/debug_debug.h"

#define OSCSERVER_DEFAULT_PATH_PRIMARY "/dmx1"
#define OSCSERVER_DEFAULT_PATH_SECONDARY OSCSERVER_DEFAULT_PATH_PRIMARY "/*"
#define OSCSERVER_DEFAULT_PATH_INFO "/2"
#define OSCSERVER_DEFAULT_PATH_BLACKOUT OSCSERVER_DEFAULT_PATH_PRIMARY "/blackout"

#define SOFTWARE_VERSION "1.0"

OscServer::OscServer()
{
    DEBUG_ENTRY();

    assert(s_this == nullptr);
    s_this = this;

    memset(s_path, 0, sizeof(s_path));
    strcpy(s_path, OSCSERVER_DEFAULT_PATH_PRIMARY);

    memset(s_path_second, 0, sizeof(s_path_second));
    strcpy(s_path_second, OSCSERVER_DEFAULT_PATH_SECONDARY);

    memset(s_path_info, 0, sizeof(s_path_info));
    strcpy(s_path_info, OSCSERVER_DEFAULT_PATH_INFO);

    memset(s_path_blackout, 0, sizeof(s_path_blackout));
    strcpy(s_path_blackout, OSCSERVER_DEFAULT_PATH_BLACKOUT);

    snprintf(os_, sizeof(os_) - 1, "[V%s] %s", SOFTWARE_VERSION, __DATE__);

    uint8_t text_length;
    model_ = hal::BoardName(text_length);
    soc_ = hal::SocName(text_length);

    if (soc_[0] == '\0')
    {
        soc_ = hal::CpuName(text_length);
    }

    DEBUG_EXIT();
}

void OscServer::SetPath(const char* path)
{
    if (*path == '/')
    {
        auto length = sizeof(s_path) - 3; // We need space for '\0' and "/*"
        strncpy(s_path, path, length);
        s_path[sizeof(s_path) - 1] = '\0';

        length = strlen(s_path);

        if (s_path[length - 1] == '/')
        {
            s_path[length - 1] = '\0';
        }

        strncpy(s_path_second, s_path, sizeof(s_path_second) - 3);

        length = strlen(s_path_second);
        assert(length < (sizeof(s_path_second) - 3));

        s_path_second[length++] = '/';
        s_path_second[length++] = '*';
        s_path_second[length] = '\0';
    }

    DEBUG_PUTS(s_path);
    DEBUG_PUTS(s_path_second);
}

void OscServer::SetPathInfo(const char* path_info)
{
    if (*path_info == '/')
    {
        strncpy(s_path_info, path_info, sizeof(s_path_info));
        s_path_info[sizeof(s_path_info) - 1] = '\0';

        const auto kLength = strlen(s_path_info);

        if (s_path_info[kLength - 1] == '/')
        {
            s_path_info[kLength - 1] = '\0';
        }
    }

    DEBUG_PUTS(s_path_info);
}

void OscServer::SetPathBlackOut(const char* path_black_out)
{
    if (*path_black_out == '/')
    {
        strncpy(s_path_blackout, path_black_out, sizeof(s_path_info));
        s_path_blackout[sizeof(s_path_info) - 1] = '\0';

        const auto kLength = strlen(s_path_blackout);

        if (s_path_blackout[kLength - 1] == '/')
        {
            s_path_blackout[kLength - 1] = '\0';
        }
    }

    DEBUG_PUTS(s_path_blackout);
}

int OscServer::GetChannel(const char* p)
{
    assert(p != nullptr);

    auto* s = const_cast<char*>(p) + strlen(s_path) + 1;
    int channel = 0;
    int i;

    for (i = 0; (i < 3) && (*s != '\0'); i++)
    {
        int c = *s;

        if ((c < '0') || (c > '9'))
        {
            return -1;
        }

        channel = channel * 10 + c - '0';
        s++;
    }

    if (channel > static_cast<int32_t>(dmxnode::kUniverseSize))
    {
        return -1;
    }

    return channel;
}

bool OscServer::IsDmxDataChanged(const uint8_t* data, uint16_t start_channel, uint32_t length)
{
    assert(data != nullptr);
    assert(length <= dmxnode::kUniverseSize);

    auto is_changed = false;
    const auto* src = data;
    auto* dst = &s_data[--start_channel];
    const auto kEnd = start_channel + length;

    assert(kEnd <= dmxnode::kUniverseSize);

    for (uint32_t i = start_channel; i < kEnd; i++)
    {
        if (*dst != *src)
        {
            *dst = *src;
            is_changed = true;
        }
        dst++;
        src++;
    }

    return is_changed;
}

void OscServer::Input(const uint8_t* buffer, uint32_t size, uint32_t from_ip, [[maybe_unused]] uint16_t from_port)
{
    auto is_dmx_data_changed = false;

    OscSimpleMessage msg(buffer, size);

    auto* udp_buffer = reinterpret_cast<const char*>(buffer);

    debug::Dump(udp_buffer, size);

    DEBUG_PRINTF("[%d] path : %s", size, osc::get_path(const_cast<char*>(udp_buffer), size));

    if (osc::is_match(udp_buffer, s_path))
    {
        const auto kArgc = msg.GetArgc();

        if ((kArgc == 1) && (msg.GetType(0) == osc::type::BLOB))
        {
            DEBUG_PUTS("Blob received");

            OSCBlob blob = msg.GetBlob(0);
            const auto kSize = static_cast<uint16_t>(blob.GetDataSize());

            if (kSize <= dmxnode::kUniverseSize)
            {
                const auto* ptr = blob.GetDataPtr();

                is_dmx_data_changed = IsDmxDataChanged(ptr, 1, kSize);

                if (is_dmx_data_changed || enable_no_change_update_)
                {
                    if ((!partial_transmission_) || (kSize == dmxnode::kUniverseSize))
                    {
                        dmxnode_output_type_->SetData<true>(0, s_data, dmxnode::kUniverseSize);
                    }
                    else
                    {
                        last_channel_ = (kSize > last_channel_ ? kSize : last_channel_);
                        dmxnode_output_type_->SetData<true>(0, s_data, last_channel_);
                    }

                    if (!is_running_)
                    {
                        is_running_ = true;
                        dmxnode_output_type_->Start(0);
                    }
                }
            }
            else
            {
                DEBUG_PUTS("Too many channels");
                return;
            }
        }
        else if ((kArgc == 2) && (msg.GetType(0) == osc::type::INT32))
        {
            auto channel = static_cast<uint16_t>(1 + msg.GetInt(0));

            if ((channel < 1) || (channel > dmxnode::kUniverseSize))
            {
                DEBUG_PRINTF("Invalid channel [%d]", channel);
                return;
            }

            uint8_t data;

            if (msg.GetType(1) == osc::type::INT32)
            {
                DEBUG_PUTS("ii received");
                data = static_cast<uint8_t>(msg.GetInt(1));
            }
            else if (msg.GetType(1) == osc::type::FLOAT)
            {
                DEBUG_PUTS("if received");
                data = static_cast<uint8_t>(msg.GetFloat(1) * dmxnode::kDmxMaxValue);
            }
            else
            {
                return;
            }

            DEBUG_PRINTF("channel = %d, data = %.2x", channel, data);

            is_dmx_data_changed = IsDmxDataChanged(&data, channel, 1);

            if (is_dmx_data_changed || enable_no_change_update_)
            {
                if (!partial_transmission_)
                {
                    dmxnode_output_type_->SetData<true>(0, s_data, dmxnode::kUniverseSize);
                }
                else
                {
                    last_channel_ = channel > last_channel_ ? channel : last_channel_;
                    dmxnode_output_type_->SetData<true>(0, s_data, last_channel_);
                }

                if (!is_running_)
                {
                    is_running_ = true;
                    dmxnode_output_type_->Start(0);
                }
            }
        }

        return;
    }

    if ((handler_ != nullptr) && (osc::is_match(udp_buffer, s_path_blackout)))
    {
        if (msg.GetType(0) != osc::type::FLOAT)
        {
            DEBUG_PUTS("No float");
            return;
        }

        if (msg.GetFloat(0) != 0)
        {
            handler_->Blackout();
            DEBUG_PUTS("Blackout");
        }
        else
        {
            handler_->Update();
            DEBUG_PUTS("Update");
        }

        return;
    }

    if (osc::is_match(udp_buffer, s_path_second))
    {
        const auto kArgc = msg.GetArgc();

        if (kArgc == 1)
        { // /path/N 'i' or 'f'
            const auto kChannel = static_cast<uint16_t>(GetChannel(udp_buffer));

            if (kChannel >= 1 && kChannel <= dmxnode::kUniverseSize)
            {
                uint8_t data;

                if (msg.GetType(0) == osc::type::INT32)
                {
                    DEBUG_PUTS("i received");
                    data = static_cast<uint8_t>(msg.GetInt(0));
                }
                else if (msg.GetType(0) == osc::type::FLOAT)
                {
                    DEBUG_PRINTF("f received %f", msg.GetFloat(0));
                    data = static_cast<uint8_t>(msg.GetFloat(0) * dmxnode::kDmxMaxValue);
                }
                else
                {
                    return;
                }

                is_dmx_data_changed = IsDmxDataChanged(&data, kChannel, 1);

                DEBUG_PRINTF(
					"Channel = %d, Data = %.2x, is_dmx_data_changed=%u, enable_no_change_update_=%u", 
                	kChannel, 
                	data, 
                	static_cast<uint32_t>(is_dmx_data_changed),
                	static_cast<uint32_t>(enable_no_change_update_)
                );

                if (is_dmx_data_changed || enable_no_change_update_)
                {
                    if (!partial_transmission_)
                    {
                        dmxnode_output_type_->SetData<true>(0, s_data, dmxnode::kUniverseSize);
                    }
                    else
                    {
                        last_channel_ = kChannel > last_channel_ ? kChannel : last_channel_;
                        dmxnode_output_type_->SetData<true>(0, s_data, last_channel_);
                    }

                    if (!is_running_)
                    {
                        is_running_ = true;
                        dmxnode_output_type_->Start(0);
                    }
                }
            }
        }

        return;
    }

    if (osc::is_match(udp_buffer, "/ping"))
    {
        OscSimpleSend send(handle_, from_ip, port_outgoing_, "/pong", nullptr);

        DEBUG_PUTS("ping received, pong sent");
        return;
    }

    if (osc::is_match(udp_buffer, s_path_info))
    {
        OscSimpleSend send_info(handle_, from_ip, port_outgoing_, "/info/os", "s", os_);
        OscSimpleSend send_model(handle_, from_ip, port_outgoing_, "/info/model", "s", model_);
        OscSimpleSend send_soc(handle_, from_ip, port_outgoing_, "/info/soc", "s", soc_);

        if (handler_ != nullptr)
        {
            handler_->Info(handle_, from_ip, port_outgoing_);
        }

        return;
    }
}
