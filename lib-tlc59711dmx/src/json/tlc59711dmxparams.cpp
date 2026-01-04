/**
 * @file tlc59711dmxparams.cpp
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

#include "common/utils/utils_enum.h"
#include "json/tlc59711dmxparams.h"
#include "json/dmxnodeparamsconst.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"
#include "tlc59711dmx.h"
#include "tlc59711.h"

namespace json
{
Tlc59711DmxParams::Tlc59711DmxParams()
{
    ConfigStore::Instance().Copy(&store_dmxled, &ConfigurationStore::dmx_led);
}

void Tlc59711DmxParams::SetType(const char* val, uint32_t len)
{
	if (len >= tlc59711::kTypesMaxNameLength) return;
	
	char type[tlc59711::kTypesMaxNameLength];
	memcpy(type, val, len);
	type[len] = '\0';
	
    store_dmxled.type = common::ToValue(tlc59711::GetType(type));
}

void Tlc59711DmxParams::SetCount(const char* val, uint32_t len)
{
    store_dmxled.count = ParseValue<uint16_t>(val, len);
}

void Tlc59711DmxParams::SetSpiSpeedHz(const char* val, uint32_t len)
{
    store_dmxled.spi_speed_hz = ParseValue<uint32_t>(val, len);
}

void Tlc59711DmxParams::SetDmxStartAddress(const char* val, uint32_t len)
{
    store_dmxled.dmx_start_address = ParseValue<uint16_t>(val, len);
}

void Tlc59711DmxParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kPixelDmxKeys);
    ConfigStore::Instance().Store(&store_dmxled, &ConfigurationStore::dmx_led);
}

void Tlc59711DmxParams::Set()
{
    auto& tcl59711dmx = *TLC59711Dmx::Get();

    tcl59711dmx.SetType(common::FromValue<tlc59711::Type>(store_dmxled.type));
    tcl59711dmx.SetCount(store_dmxled.count);
	tcl59711dmx.SetSpiSpeedHz(store_dmxled.spi_speed_hz);
	tcl59711dmx.SetDmxStartAddress(store_dmxled.dmx_start_address);

#ifndef NDEBUG
	tcl59711dmx.Print();
    Dump();
#endif
}

void Tlc59711DmxParams::Dump()
{
    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DmxLedParamsConst::kFileName);
    printf(" %s=%s\n", DmxLedParamsConst::kType.name, tlc59711::GetType(static_cast<tlc59711::Type>(store_dmxled.type)));
    printf(" %s=%u\n", DmxLedParamsConst::kCount.name, store_dmxled.count);
    printf(" %s=%u\n", DmxLedParamsConst::kSpiSpeedHz.name, store_dmxled.spi_speed_hz);
    printf(" %s=%d\n", DmxNodeParamsConst::kDmxStartAddress.name, store_dmxled.dmx_start_address);
}
} // namespace json