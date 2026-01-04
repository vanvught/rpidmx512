/**
 * @file json_config_tlc59711.cpp
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

#include "json/dmxledparamsconst.h"
#include "json/dmxnodeparamsconst.h"
#include "json/tlc59711dmxparams.h"
#include "json/json_helpers.h"
#include "tlc59711dmx.h"
#include "tlc59711.h"

namespace json::config
{
uint32_t GetTlc59711Dmx(char* buffer, uint32_t length)
{
    auto& tlc59711_dmx = *TLC59711Dmx::Get();

	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
	    doc[DmxLedParamsConst::kType.name] = tlc59711::GetType(tlc59711_dmx.GetType());
	    doc[DmxLedParamsConst::kCount.name] = tlc59711_dmx.GetCount();
	    doc[DmxLedParamsConst::kSpiSpeedHz.name] = tlc59711_dmx.GetSpiSpeedHz();
	    doc[DmxNodeParamsConst::kDmxStartAddress.name] = tlc59711_dmx.GetDmxStartAddress();
    });
}

void SetTlc59711Dmx(const char* buffer, uint32_t buffer_size)
{
    ::json::Tlc59711DmxParams tlc59711dmx_params;
    tlc59711dmx_params.Store(buffer, buffer_size);
	tlc59711dmx_params.Set();
}
} // namespace json::config