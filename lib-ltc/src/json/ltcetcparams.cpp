/**
 * @file ltcetcparams.cpp
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

#include <cstdint>
#include <cstring>

#include "json/ltcetcparams.h"
#include "common/utils/utils_enum.h"
#include "json/ltcetcparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "ip4/ip4_helpers.h"
#include "configstore.h"
#include "configurationstore.h"
#include "ltcetc.h"

namespace json
{
LtcEtcParams::LtcEtcParams()
{
    ConfigStore::Instance().Copy(&store_ltcetc, &ConfigurationStore::ltc_etc);
}

void LtcEtcParams::SetDestinationIp(const char* val, uint32_t len)
{
    store_ltcetc.destination_ip = net::ParseIpString(val, len);
}

void LtcEtcParams::SetDestinationPort(const char* val, uint32_t len)
{
    if (len > 3) store_ltcetc.destination_port = ParseValue<uint16_t>(val, len);
}

void LtcEtcParams::SetSourceMulticastIp(const char* val, uint32_t len)
{
    store_ltcetc.source_multicast_ip = net::ParseIpString(val, len);
}

void LtcEtcParams::SetSourcePort(const char* val, uint32_t len)
{
    if (len > 3) store_ltcetc.source_port = ParseValue<uint16_t>(val, len);
}

void LtcEtcParams::SetUdpTerminator(const char* val, uint32_t len)
{
    if (len < ltcetc::kMaxNameLength)
    {
        char a[ltcetc::kMaxNameLength];
        memcpy(a, val, len);
        a[len] = '\0';

        store_ltcetc.udp_terminator = common::ToValue(ltcetc::GetUdpTerminator(a));
    }
}

void LtcEtcParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kLtcEtcKeys);
    ConfigStore::Instance().Store(&store_ltcetc, &ConfigurationStore::ltc_etc);

#ifndef NDEBUG
    Dump();
#endif
}

void LtcEtcParams::Set()
{
    auto* etc = LtcEtc::Get();
    assert(etc != nullptr);

    etc->SetDestinationIp(store_ltcetc.destination_ip);
    etc->SetDestinationPort(store_ltcetc.destination_port);
    etc->SetSourceMulticastIp(store_ltcetc.source_multicast_ip);
    etc->SetSourcePort(store_ltcetc.source_port);
    etc->SetUdpTerminator(common::FromValue<ltcetc::UdpTerminator>(store_ltcetc.udp_terminator));

#ifndef NDEBUG
    etc->Print();
    Dump();
#endif
}

void LtcEtcParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::LtcEtcParamsConst::kFileName);
}
} // namespace json
