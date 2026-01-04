/**
 * @file rdm_manufacturer_pid.cpp
 *
 */
/* Copyright (C) 2023-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#if defined(DEBUG_PIXELDMX)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>

#include "rdmhandler.h"
#include "rdm_e120.h"
#include "pixeltype.h"
#include "pixeldmxstore.h"
 #include "firmware/debug/debug_debug.h"
#include "pixeldmxconfiguration.h"
#include "common/utils/utils_enum.h"
#include "pixeloutput.h"

#if !defined(OUTPUT_DMX_PIXEL)
#error
#endif

using E120_MANUFACTURER_PIXEL_TYPE = rdmhandler::ManufacturerPid<0x8500>;
using E120_MANUFACTURER_PIXEL_COUNT = rdmhandler::ManufacturerPid<0x8501>;
using E120_MANUFACTURER_PIXEL_GROUPING_COUNT = rdmhandler::ManufacturerPid<0x8502>;
using E120_MANUFACTURER_PIXEL_MAP = rdmhandler::ManufacturerPid<0x8503>;

struct PixelType
{
    static constexpr char kDescription[] = "Pixel type";
};

struct PixelCount
{
    static constexpr char kDescription[] = "Pixel count";
};

struct PixelGroupingCount
{
    static constexpr char kDescription[] = "Pixel grouping count";
};

struct PixelMap
{
    static constexpr char kDescription[] = "Pixel map";
};

constexpr char PixelType::kDescription[];
constexpr char PixelCount::kDescription[];
constexpr char PixelGroupingCount::kDescription[];
constexpr char PixelMap::kDescription[];

const rdmhandler::ParameterDescription RDMHandler::PARAMETER_DESCRIPTIONS[] = {
		  { E120_MANUFACTURER_PIXEL_TYPE::kCode,
		    rdmhandler::kDeviceDescriptionMaxLength,
			E120_DS_ASCII,
#if defined (CONFIG_RDM_MANUFACTURER_PIDS_SET)
			E120_CC_GET_SET,
#else
			E120_CC_GET,
#endif
			0,
			E120_UNITS_NONE,
			E120_PREFIX_NONE,
			0,
			0,
			0,
			rdmhandler::Description<PixelType, sizeof(PixelType::kDescription)>::kValue,
			RDMHandler::PdlParameterDescription(sizeof(PixelType::kDescription))
		  },
		  { E120_MANUFACTURER_PIXEL_COUNT::kCode,
			2,
			E120_DS_UNSIGNED_WORD,
#if defined (CONFIG_RDM_MANUFACTURER_PIDS_SET)
			E120_CC_GET_SET,
#else
			E120_CC_GET,
#endif
			0,
			E120_UNITS_NONE,
			E120_PREFIX_NONE,
			0,
			__builtin_bswap32(pixel::defaults::kCount),
			__builtin_bswap32(pixel::max::ledcount::RGB),
			rdmhandler::Description<PixelCount, sizeof(PixelCount::kDescription)>::kValue,
			RDMHandler::PdlParameterDescription(sizeof(PixelCount::kDescription))
		  },
		  { E120_MANUFACTURER_PIXEL_GROUPING_COUNT::kCode,
			2,
			E120_DS_UNSIGNED_WORD,
#if defined (CONFIG_RDM_MANUFACTURER_PIDS_SET)
			E120_CC_GET_SET,
#else
			E120_CC_GET,
#endif
			0,
			E120_UNITS_NONE,
			E120_PREFIX_NONE,
			0,
			__builtin_bswap32(pixel::defaults::kCount),
			__builtin_bswap32(pixel::max::ledcount::RGB),
			rdmhandler::Description<PixelGroupingCount, sizeof(PixelGroupingCount::kDescription)>::kValue,
			RDMHandler::PdlParameterDescription(sizeof(PixelGroupingCount::kDescription))
		  },
		  { E120_MANUFACTURER_PIXEL_MAP::kCode,
			rdmhandler::kDeviceDescriptionMaxLength,
			E120_DS_ASCII,
#if defined (CONFIG_RDM_MANUFACTURER_PIDS_SET)
			E120_CC_GET_SET,
#else
			E120_CC_GET,
#endif
			0,
			E120_UNITS_NONE,
			E120_PREFIX_NONE,
			0,
			0,
			0,
			rdmhandler::Description<PixelMap, sizeof(PixelMap::kDescription)>::kValue,
			RDMHandler::PdlParameterDescription(sizeof(PixelMap::kDescription))
		  }
  };

uint32_t RDMHandler::GetParameterDescriptionCount() const
{
    return sizeof(RDMHandler::PARAMETER_DESCRIPTIONS) / sizeof(RDMHandler::PARAMETER_DESCRIPTIONS[0]);
}

namespace rdmhandler {
bool HandleManufactureerPidGet(uint16_t pid, [[maybe_unused]] const ManufacturerParamData* in, ManufacturerParamData* out, uint16_t& reason)
{
    DEBUG_PRINTF("pid=%x", __builtin_bswap16(pid));

    auto& pixeldmx_configuration = PixelDmxConfiguration::Get();

    switch (pid)
    {
        case E120_MANUFACTURER_PIXEL_TYPE::kCode:
        {
            const auto* string = ::pixel::GetType(pixeldmx_configuration.GetType());
            out->nPdl = static_cast<uint8_t>(strlen(string));
            memcpy(out->pParamData, string, out->nPdl);
            return true;
        }
        case E120_MANUFACTURER_PIXEL_COUNT::kCode:
        {
            const auto kCount = pixeldmx_configuration.GetCount();
            out->nPdl = 2;
            out->pParamData[0] = static_cast<uint8_t>(kCount >> 8);
            out->pParamData[1] = static_cast<uint8_t>(kCount);
            return true;
        }
        case E120_MANUFACTURER_PIXEL_GROUPING_COUNT::kCode:
        {
            const auto kGrouingCount = pixeldmx_configuration.GetGroupingCount();
            out->nPdl = 2;
            out->pParamData[0] = static_cast<uint8_t>(kGrouingCount >> 8);
            out->pParamData[1] = static_cast<uint8_t>(kGrouingCount);
            return true;
        }
        case E120_MANUFACTURER_PIXEL_MAP::kCode:
        {
            const auto* string = ::pixel::GetMap(pixeldmx_configuration.GetMap());
            out->nPdl = static_cast<uint8_t>(strlen(string));
            memcpy(out->pParamData, string, out->nPdl);
            return true;
        }
        default:
            break;
    }

    reason = E120_NR_UNKNOWN_PID;
    return false;
}

#if defined(CONFIG_RDM_MANUFACTURER_PIDS_SET)
bool HandleManufactureerPidSet(bool is_broadcast, uint16_t pid, const ParameterDescription& parameter_description, const ManufacturerParamData* in,
                               [[maybe_unused]] ManufacturerParamData* out, uint16_t& reason)
{
    DEBUG_PRINTF("pid=%x", __builtin_bswap16(pid));

    if (is_broadcast)
    {
        return false;
    }

    auto& pixeldmx_configuration = PixelDmxConfiguration::Get();

    switch (pid)
    {
        case E120_MANUFACTURER_PIXEL_COUNT::kCode:
        {
            if (in->nPdl == 2)
            {
                const uint16_t kCount = in->pParamData[1] | in->pParamData[0] << 8;

                if ((kCount < parameter_description.min_value) && (kCount > parameter_description.max_value))
                {
                    reason = E120_NR_DATA_OUT_OF_RANGE;
                    return false;
                }

                pixeldmx_configuration.SetCount(kCount);
                dmxled_store::SaveCount(kCount);
                PixelOutputType::Get()->ApplyConfiguration();
                return true;
            }

            reason = E120_NR_FORMAT_ERROR;
            return false;
        }
        case E120_MANUFACTURER_PIXEL_GROUPING_COUNT::kCode:
        {
            if (in->nPdl == 2)
            {
                const uint16_t kGrouingCount = in->pParamData[1] | in->pParamData[0] << 8;

                if ((kGrouingCount < parameter_description.min_value) && (kGrouingCount > parameter_description.max_value))
                {
                    reason = E120_NR_DATA_OUT_OF_RANGE;
                    return false;
                }

                pixeldmx_configuration.SetGroupingCount(kGrouingCount);
                dmxled_store::SaveGroupingCount(kGrouingCount);
                PixelOutputType::Get()->ApplyConfiguration();
                return true;
            }

            reason = E120_NR_FORMAT_ERROR;
            return false;
        }
        case E120_MANUFACTURER_PIXEL_MAP::kCode:
        {
            if (in->nPdl == 3)
            {
                const auto kMap = ::pixel::GetMap(reinterpret_cast<const char*>(in->pParamData));

                if (kMap == pixel::Map::UNDEFINED)
                {
                    reason = E120_NR_DATA_OUT_OF_RANGE;
                    return false;
                }

                pixeldmx_configuration.SetMap(kMap);
                dmxled_store::SaveMap(common::ToValue(kMap));
                PixelOutputType::Get()->ApplyConfiguration();
                return true;
            }

            reason = E120_NR_FORMAT_ERROR;
            return false;
        }
        default:
            break;
    }

    reason = E120_NR_UNKNOWN_PID;
    return false;
}
#endif
} // namespace rdmhandler