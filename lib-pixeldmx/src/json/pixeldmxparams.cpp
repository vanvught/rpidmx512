/**
 * @file pixeldmxparams.cpp
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
#include <algorithm>

#include "pixeltype.h"
#include "json/pixeldmxparams.h"
#include "json/json_parser.h"
#include "json/json_parsehelper.h"
#include "configstore.h"
#include "configurationstore.h"
#include "common/utils/utils_flags.h"
#include "pixelconfiguration.h"
#include "pixeldmxconfiguration.h"
#include "common/utils/utils_enum.h"
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
#include "gamma/gamma_tables.h"
#endif
#include "dmxnode_outputtype.h"
#include "firmware/pixeldmx/show.h"
#include "dmxnode.h"
#include "dmxnode_nodetype.h"
#include "pixeltestpattern.h"

static constexpr uint32_t kConfigMaxPorts = CONFIG_DMXNODE_PIXEL_MAX_PORTS;

using common::store::dmxled::Flags;

namespace json
{
PixelDmxParams::PixelDmxParams()
{
    ConfigStore::Instance().Copy(&store_dmxled, &ConfigurationStore::dmx_led);
}

void PixelDmxParams::SetType(const char* val, uint32_t len)
{
    if (len < pixel::kTypesMaxNameLength)
    {
        char type[pixel::kTypesMaxNameLength];
        memcpy(type, val, len);
        type[len] = '\0';

        store_dmxled.type = common::ToValue(pixel::GetType(type));
    }
}

void PixelDmxParams::SetMap(const char* val, uint32_t len)
{
    if (len == 0)
    {
        store_dmxled.map = common::ToValue(pixel::Map::UNDEFINED);
        return;
    }

    if (len == 3)
    {
        char map[4];
        memcpy(map, val, 3);
        map[3] = '\0';

        store_dmxled.map = common::ToValue(pixel::GetMap(map));
    }
}

void PixelDmxParams::SetCount(const char* val, uint32_t len)
{
    store_dmxled.count = ParseValue<uint16_t>(val, len);
}

void PixelDmxParams::SetGroupingCount(const char* val, uint32_t len)
{
    store_dmxled.grouping_count = ParseValue<uint16_t>(val, len);
}

void PixelDmxParams::SetLowCode(const char* val, uint32_t len)
{
    store_dmxled.low_code = pixel::ConvertTxH(json::Atof(val, len));
}

void PixelDmxParams::SetHighCode(const char* val, uint32_t len)
{
    store_dmxled.high_code = pixel::ConvertTxH(json::Atof(val, len));
}

#if defined(OUTPUT_DMX_PIXEL_MULTI)
void PixelDmxParams::SetActiveOutputs(const char* val, uint32_t len)
{
    store_dmxled.active_outputs = ParseValue<uint8_t>(val, len);
}
#endif

void PixelDmxParams::SetTestPattern(const char* val, uint32_t len)
{
    if (len == 1) store_dmxled.test_pattern = ParseValue<uint8_t>(val, len);
}

void PixelDmxParams::SetSpiSpeedHz(const char* val, uint32_t len)
{
    store_dmxled.spi_speed_hz = ParseValue<uint32_t>(val, len);
}

void PixelDmxParams::SetGlobalBrightness(const char* val, uint32_t len)
{
    store_dmxled.global_brightness = ParseValue<uint8_t>(val, len);
}

void PixelDmxParams::SetStartUniPort(const char* key, uint32_t key_len, const char* val, uint32_t val_len)
{
    const char kSuffix = key[key_len - 1];
    auto index = static_cast<uint32_t>(kSuffix - '1');

    if (key_len == 17)
    {
        index += 10;
    }

    auto v = ParseValue<uint16_t>(val, val_len);
    store_dmxled.start_universe[index] = v;
}

#if defined(RDM_RESPONDER)
void PixelDmxParams::SetDmxStartAddress(const char* val, uint32_t len)
{
    store_dmxled.dmx_start_address = ParseValue<uint16_t>(val, len);
}
#endif

#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
void PixelDmxParams::SetGammaCorrection(const char* val, uint32_t len)
{
    ParseAndApply<uint8_t>(val, len, [](uint8_t v) { store_dmxled.flags = common::SetFlagValue(store_dmxled.flags, Flags::Flag::kEnableGamma, v != 0); });
}

void PixelDmxParams::SetGammaValue(const char* val, uint32_t len)
{
    if ((len == 1) && (val[0] == '0'))
    {
        store_dmxled.gamma_value = 0;
        return;
    }

    if (len != 3)
    {
        return;
    }

    const auto kV = gamma::GetValidValue(static_cast<uint32_t>(json::Atof(val, 3) * 10));
    store_dmxled.gamma_value = static_cast<uint8_t>(kV);
}
#endif

void PixelDmxParams::Store(const char* buffer, uint32_t buffer_size)
{
    ParseJsonWithTable(buffer, buffer_size, kPixelDmxKeys);
    ConfigStore::Instance().Store(&store_dmxled, &ConfigurationStore::dmx_led);

#ifndef NDEBUG
    Dump();
#endif
}

void PixelDmxParams::Set()
{
    auto& pixel_configuration = PixelConfiguration::Get();

    pixel_configuration.SetType(static_cast<pixel::Type>(store_dmxled.type));
    pixel_configuration.SetMap(static_cast<pixel::Map>(store_dmxled.map));
    pixel_configuration.SetCount(store_dmxled.count);
    pixel_configuration.SetLowCode(store_dmxled.low_code);
    pixel_configuration.SetHighCode(store_dmxled.high_code);
    pixel_configuration.SetClockSpeedHz(store_dmxled.spi_speed_hz);
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
    pixel_configuration.SetEnableGammaCorrection(common::IsFlagSet(store_dmxled.flags, Flags::Flag::kEnableGamma));
    pixel_configuration.SetGammaTable(store_dmxled.gamma_value);
#endif
    auto& pixel_dmx_configuration = PixelDmxConfiguration::Get();
    pixel_dmx_configuration.SetGroupingCount(store_dmxled.grouping_count);
#if defined(OUTPUT_DMX_PIXEL_MULTI)
    pixel_dmx_configuration.SetOutputPorts(store_dmxled.active_outputs);
#endif
#if !defined(OUTPUT_DMX_PIXEL_MULTI)
    pixel_dmx_configuration.SetDmxStartAddress(store_dmxled.dmx_start_address);
#endif

    DmxPixelOutputType::Get().ApplyConfiguration();

#if defined(DMXNODE_TYPE_ARTNET) || defined(DMXNODE_TYPE_E131)
    const auto kUniverses = pixel_dmx_configuration.GetUniverses();
    const auto kPixelOutputPorts = pixel_dmx_configuration.GetOutputPorts();

    uint32_t protocol_port_index = 0;

    for (uint32_t pixel_port_index = 0; pixel_port_index < kPixelOutputPorts; pixel_port_index++)
    {
        const auto kStartUniverse = ConfigStore::Instance().DmxLedIndexedGetStartUniverse(pixel_port_index);

        for (uint32_t universe = 0; universe < kUniverses; universe++)
        {
            if (kStartUniverse != 0)
            {
                DmxNodeNodeType::Get()->SetUniverse(protocol_port_index, static_cast<uint16_t>(kStartUniverse + universe));
                DmxNodeNodeType::Get()->SetDirection(protocol_port_index, dmxnode::PortDirection::kOutput);

                char label[dmxnode::kLabelNameLength];
                snprintf(label, dmxnode::kLabelNameLength - 1, "Pixel %c -> %u:%u", static_cast<char>('A' + pixel_port_index), protocol_port_index,
                         kStartUniverse + universe);
                DmxNode::Instance().SetShortName(protocol_port_index, label);
            }
            protocol_port_index++;
        }
    }

    for (; protocol_port_index < dmxnode::kMaxPorts; protocol_port_index++)
    {
        DmxNodeNodeType::Get()->SetDirection(protocol_port_index, dmxnode::PortDirection::kDisable);
        DmxNode::Instance().SetShortNameDefault(protocol_port_index);
    }
#endif

#ifndef NDEBUG
    pixel_dmx_configuration.Print();
    Dump();
#if defined(DMXNODE_TYPE_ARTNET) || defined(DMXNODE_TYPE_E131)
    DmxNodeNodeType::Get()->Print();
#endif
#endif
    const auto kTestPattern = common::FromValue<pixelpatterns::Pattern>(store_dmxled.test_pattern);

    if (kTestPattern != PixelTestPattern::Get()->GetPattern())
    {
        const auto kIsSet = PixelTestPattern::Get()->SetPattern(kTestPattern);

        if (kIsSet)
        {
            PixelOutputType::Get()->Blackout();
#if defined(DMXNODE_TYPE_ARTNET) || defined(DMXNODE_TYPE_E131)
            if (static_cast<pixelpatterns::Pattern>(kTestPattern) == pixelpatterns::Pattern::kNone)
            {
                DmxNodeNodeType::Get()->SetOutput(&DmxNodeOutputType::Get());
            }
            else
            {
                DmxNodeNodeType::Get()->SetOutput(nullptr);
            }
#endif
        }
    }

    common::firmware::pixeldmx::Show(7, kTestPattern);
}

void PixelDmxParams::Dump()
{
    static const auto kMaxStartUniverses = std::min(kConfigMaxPorts, common::store::dmxled::kMaxUniverses);

    printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, json::DmxLedParamsConst::kFileName);
    printf(" %s=%s [%u]\n", json::DmxLedParamsConst::kType.name, pixel::GetType(common::FromValue<pixel::Type>(store_dmxled.type)), store_dmxled.type);
    printf(" %s=%.2f [0x%X]\n", DmxLedParamsConst::kT0H.name, pixel::ConvertTxH(store_dmxled.low_code), store_dmxled.low_code);
    printf(" %s=%.2f [0x%X]\n", DmxLedParamsConst::kT1H.name, pixel::ConvertTxH(store_dmxled.high_code), store_dmxled.high_code);
    printf(" %s=%s\n", DmxLedParamsConst::kMap.name, pixel::GetMap(static_cast<pixel::Map>(store_dmxled.map)));
    printf(" %s=%u\n", DmxLedParamsConst::kCount.name, store_dmxled.count);
    printf(" %s=%u\n", DmxLedParamsConst::kGroupingCount.name, store_dmxled.grouping_count);
    for (uint32_t i = 0; i < kMaxStartUniverses; i++)
    {
        printf(" %s=%d\n", PixelDmxParamsConst::kStartUniPort[i].name, store_dmxled.start_universe[i]);
    }
#if defined(OUTPUT_DMX_PIXEL_MULTI)
    printf(" %s=%d\n", DmxLedParamsConst::kActiveOutputPorts.name, store_dmxled.active_outputs);
#endif
    printf(" %s=%d\n", DmxLedParamsConst::kTestPattern.name, store_dmxled.test_pattern);
    printf(" %s=%u\n", DmxLedParamsConst::kSpiSpeedHz.name, store_dmxled.spi_speed_hz);
    printf(" %s=%d\n", DmxLedParamsConst::kGlobalBrightness.name, store_dmxled.global_brightness);
#if defined(RDM_RESPONDER)
    printf(" %s=%d\n", PixelDmxParamsConst::kDmxStartAddress.name, store_dmxled.dmx_start_address);
#endif
#if defined(CONFIG_PIXELDMX_ENABLE_GAMMATABLE)
    printf(" %s=%d\n", DmxLedParamsConst::kGammaCorrection.name, common::IsFlagSet(store_dmxled.flags, Flags::Flag::kEnableGamma));
    printf(" %s=%1.1f [%u]\n", DmxLedParamsConst::kGammaValue.name, static_cast<float>(store_dmxled.gamma_value) / 10.0f, store_dmxled.gamma_value);
#endif
}
} // namespace json
