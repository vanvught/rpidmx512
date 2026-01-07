/**
 * @file json_config_ltcetc.cpp
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
#include <cassert>

#include "ip4/ip4_helpers.h"
#include "json/json_helpers.h"
#include "ltcetc.h"
#include "json/ltcetcparamsconst.h"
#include "json/ltcetcparams.h"

namespace json::config
{
uint32_t GetLtcEtc(char* buffer, uint32_t length)
{
	auto* etc = LtcEtc::Get();
	assert(etc != nullptr);
	
	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
		char ip[net::kIpBufferSize];
		doc[LtcEtcParamsConst::kDestinationIp.name] = net::FormatIp(etc->GetDestinationIp(), ip);
		doc[LtcEtcParamsConst::kDestinationPort.name] = etc->GetDestinationPort();
    	doc[LtcEtcParamsConst::kSourceMulticastIp.name] = net::FormatIp(etc->GetSourceMulticastIp(), ip);
    	doc[LtcEtcParamsConst::kSourcePort.name] = etc->GetSourcePort();
    	doc[LtcEtcParamsConst::kUdpTerminator.name] = ltcetc::GetUdpTerminator(etc->GetUdpTerminator());
    });
}

void SetLtcEtc(const char* buffer, uint32_t buffer_size)
{
    ::json::LtcEtcParams ltcetc_params;
    ltcetc_params.Store(buffer, buffer_size);
    ltcetc_params.Set();
}
} // namespace json::config
