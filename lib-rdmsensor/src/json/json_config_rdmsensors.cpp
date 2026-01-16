/**
 * @file json_config_rdmsensors.cpp
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

#include "json/rdmsensorsparams.h"
#include "json/json_helpers.h"
#include "configurationstore.h"
#include "configstore.h"
#include "common/utils/utils_hex.h"

namespace json::config
{
uint32_t GetRdmSensors(char* buffer, uint32_t length)
{
	return json::helpers::Serialize(buffer, length, [&](JsonDoc& doc) {
	    char a[32];
	    for (size_t types = 0; types < json::RdmSensorsParams::KeysSize(); types++)
	    {
	        a[0] = '[';
	        uint32_t k = 1;
	        for (uint32_t j = 0; j < common::store::rdm::sensors::kMaxDevices; j++)
	        {
	            if ((k + 3) > 31) break;
	            const auto kType = ConfigStore::Instance().RdmSensorsIndexedGetType(j);
	            if (kType == types)
	            {
	                const auto kAddress = ConfigStore::Instance().RdmSensorsIndexedGetAddress(j);
	                if (kAddress != 0)
	                {
	                    a[k++] = common::hex::ToCharLowercase(kAddress >> 4);
	                    a[k++] = common::hex::ToCharLowercase(kAddress);
	                    a[k++] = ',';
	                }
	            }
	        }
	
	        if (a[k - 1] == ',') k -= 1;
	
	        a[k++] = ']';
	        a[k] = '\0';
	
	        const auto& keys = json::RdmSensorsParams::Keys();
	        doc[keys[types].GetName()] = a;
	    }
    });
}

void SetRdmSensors(const char* buffer, uint32_t buffer_size)
{
    ::json::RdmSensorsParams rdmsensors_params;
    rdmsensors_params.Store(buffer, buffer_size);
}
} // namespace json::config