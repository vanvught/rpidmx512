/**
 * @file pixeldmxparamsrdm.h
 *
 */
/* Copyright (C) 2021-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#ifndef PIXELDMXPARAMSRDM_H_
#define PIXELDMXPARAMSRDM_H_

#include <cstdint>
#include <cassert>

#include "dmxnode.h"
#include "dmxnodeoutputrdmpixel.h"

#include "debug.h"

namespace pixeldmx::paramsdmx {
enum class SlotInfo {
	TYPE, COUNT, GROUPING_COUNT, MAP, TEST_PATTERN, PROGRAM, LAST
};
static constexpr auto DMX_FOOTPRINT = static_cast<uint16_t>(SlotInfo::LAST);
} // namespace pixeldmx::paramsdmx


class PixelDmxParamsRdm final : public DmxNodeOutputRdmPixel {
public:
	PixelDmxParamsRdm() {
		DEBUG_ENTRY
		DEBUG_EXIT
	}

	void Start([[maybe_unused]] const uint32_t nPortIndex) {}
	void Stop([[maybe_unused]] const uint32_t nPortIndex) {}

	void SetData(const uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true) override;

	uint16_t GetDmxFootprint() override {
		return pixeldmx::paramsdmx::DMX_FOOTPRINT;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, dmxnode::SlotInfo &slotInfo) override {
		if (nSlotOffset >= pixeldmx::paramsdmx::DMX_FOOTPRINT) {
			return false;
		}

		slotInfo.nType = 0x00;			// ST_PRIMARY
		slotInfo.nCategory = 0xFFFF;	// SD_UNDEFINED;

		return true;
	}

	void Display(const uint8_t *pData) __attribute__((weak));

private:
	uint8_t m_Data { 0 };
};

#endif /* PIXELDMXPARAMSRDM_H_ */
