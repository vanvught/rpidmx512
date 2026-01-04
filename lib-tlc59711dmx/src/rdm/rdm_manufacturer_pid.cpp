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

#include "tlc59711.h"
#if defined(DEBUG_PIXELDMX)
#undef NDEBUG
#endif

#include <cstdint>
#include <cstring>

#include "rdmhandler.h"
#include "rdm_e120.h"
#include "tlc59711dmx.h"
#if !defined(OUTPUT_DMX_TLC59711)
#error
#endif

using E120_MANUFACTURER_PIXEL_TYPE = rdmhandler::ManufacturerPid<0x8500>;
using E120_MANUFACTURER_PIXEL_COUNT =rdmhandler::ManufacturerPid<0x8501>;

struct PixelType
{
    static constexpr char kDescription[] = "Pixel type";
};

struct PixelCount
{
    static constexpr char kDescription[] = "Pixel count";
};

constexpr char PixelType::kDescription[];
constexpr char PixelCount::kDescription[];

const rdmhandler::ParameterDescription RDMHandler::PARAMETER_DESCRIPTIONS[] = {
		  { E120_MANUFACTURER_PIXEL_TYPE::kCode,
		    rdmhandler::kDeviceDescriptionMaxLength,
			E120_DS_ASCII,
			E120_CC_GET,
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
			E120_DS_UNSIGNED_DWORD,
			E120_CC_GET,
			0,
			E120_UNITS_NONE,
			E120_PREFIX_NONE,
			0,
			__builtin_bswap32(4),
			__builtin_bswap32(4),
			rdmhandler::Description<PixelCount, sizeof(PixelCount::kDescription)>::kValue,
			RDMHandler::PdlParameterDescription(sizeof(PixelCount::kDescription))
		  }
  };

uint32_t RDMHandler::GetParameterDescriptionCount() const {
	return sizeof(RDMHandler::PARAMETER_DESCRIPTIONS) / sizeof(RDMHandler::PARAMETER_DESCRIPTIONS[0]);
}

namespace rdmhandler {
bool HandleManufactureerPidGet(const uint16_t nPid, [[maybe_unused]] const ManufacturerParamData *pIn, ManufacturerParamData *pOut, uint16_t& nReason) {
	switch (nPid) {
	case E120_MANUFACTURER_PIXEL_TYPE::kCode: {
		const auto *string = tlc59711::GetType(TLC59711Dmx::Get()->GetType());
		pOut->nPdl = static_cast<uint8_t>(strlen(string));
		memcpy(pOut->pParamData, string, pOut->nPdl);
		return true;
	}
	case E120_MANUFACTURER_PIXEL_COUNT::kCode: {
		const auto kCount = TLC59711Dmx::Get()->GetCount();
		pOut->nPdl = 4;
		pOut->pParamData[0] = static_cast<uint8_t>(kCount >> 24);
		pOut->pParamData[1] = static_cast<uint8_t>(kCount >> 16);
		pOut->pParamData[2] = static_cast<uint8_t>(kCount >> 8);
		pOut->pParamData[3] = static_cast<uint8_t>(kCount);
		return true;
	}
	default:
		break;
	}

	nReason = E120_NR_UNKNOWN_PID;
	return false;
}
}  // namespace rdmhandler
