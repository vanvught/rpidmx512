/**
 * @file showfileparams.cpp
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
 
 #include <cstdint>

#include "showfile.h"
#include "json/oscparamsconst.h"
#include "json/showfileparams.h"
#include "json/showfileparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"

using common::store::showfile::Flags;

namespace json
{
ShowFileParams::ShowFileParams()
{
    ConfigStore::Instance().Copy(&store_showfile, &ConfigurationStore::show_file);
}

void ShowFileParams::SetShow(const char* val, uint32_t len)
{
    if (len < 3) store_showfile.show = ParseValue<uint8_t>(val, len);
}

void ShowFileParams::SetOptionAutoPlay(const char* val, uint32_t len)
{
    if (len == 1) store_showfile.flags = common::SetFlagValue(store_showfile.flags, Flags::Flag::kOptionAutoPlay, val[0] != '0');
}

void ShowFileParams::SetOptionLoop(const char* val, uint32_t len)
{
    if (len == 1) store_showfile.flags = common::SetFlagValue(store_showfile.flags, Flags::Flag::kOptionLoop, val[0] != '0');
}

void ShowFileParams::SetIncomingPort(const char* val, uint32_t len)
{
    if (len > 3) store_showfile.osc_port_incoming = ParseValue<uint16_t>(val, len);
}

void ShowFileParams::SetOutgoingPort(const char* val, uint32_t len)
{
    if (len > 3) store_showfile.osc_port_outgoing = ParseValue<uint16_t>(val, len);
}

void ShowFileParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kShowFileKeys);
    ConfigStore::Instance().Store(&store_showfile, &ConfigurationStore::show_file);
}

void ShowFileParams::Set()
{
    auto& showfile = ShowFile::Instance();

    showfile.SetPlayerShowFileCurrent(store_showfile.show);
    // Options
    showfile.SetAutoStart(common::IsFlagSet(store_showfile.flags, Flags::Flag::kOptionAutoPlay));
    showfile.DoLoop(common::IsFlagSet(store_showfile.flags, Flags::Flag::kOptionLoop));
#if defined(CONFIG_SHOWFILE_ENABLE_OSC)
    showfile.SetOscPortIncoming(store_showfile.osc_port_incoming);
    showfile.SetOscPortOutgoing(store_showfile.osc_port_outgoing);
#endif

    if (showfile.IsAutoStart())
    {
        showfile.Play();
    }

#ifndef NDEBUG
    Dump();
#endif
}

void ShowFileParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::ShowFileParamsConst::kFileName);
    printf(" %s=%u\n", json::ShowFileParamsConst::kShow.name, store_showfile.show);
    printf(" %s=%u\n", json::ShowFileParamsConst::kOptionAutoPlay.name, common::IsFlagSet(store_showfile.flags, Flags::Flag::kOptionAutoPlay));
    printf(" %s=%u\n", json::ShowFileParamsConst::kOptionLoop.name, common::IsFlagSet(store_showfile.flags, Flags::Flag::kOptionLoop));
#if defined(CONFIG_SHOWFILE_ENABLE_OSC)
    printf(" %s=%u\n", OscParamsConst::kIncomingPort.name, store_showfile.osc_port_incoming);
    printf(" %s=%u\n", OscParamsConst::kOutgoingPort.name, store_showfile.osc_port_outgoing);
#endif
}
} // namespace json
