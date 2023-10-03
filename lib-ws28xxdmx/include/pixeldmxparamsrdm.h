/**
 * @file pixeldmxparamsrdm.h
 *
 */
/* Copyright (C) 2021-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include "pixeldmxstore.h"
#include "lightset.h"

namespace pixeldmx {
namespace paramsdmx {
enum class SlotInfo {
	TYPE, COUNT, GROUPING_COUNT, MAP, TEST_PATTERN, PROGRAM, LAST
};
static constexpr auto DMX_FOOTPRINT = static_cast<uint16_t>(SlotInfo::LAST);
}  // namespace paramsdmx
}  // namespace pixeldmx

class PixelDmxParamsRdm: public LightSet {
public:
	PixelDmxParamsRdm(PixelDmxStore *pWS28xxDmxStore);

	void Start(const uint32_t nPortIndex) override;
	void Stop(const uint32_t nPortIndex) override;
	void SetData(uint32_t nPortIndex, const uint8_t *pData, uint32_t nLength, const bool doUpdate = true) override;
	void Sync(__attribute__((unused)) const uint32_t nPortIndex) override {}
	void Sync(__attribute__((unused)) const bool doForce) override {}

	bool SetDmxStartAddress(__attribute__((unused)) uint16_t nDmxStartAddress) override {
		return false;
	}

	uint16_t GetDmxStartAddress() override {
		return 1;
	}

	uint16_t GetDmxFootprint() override {
		return pixeldmx::paramsdmx::DMX_FOOTPRINT;
	}

	bool GetSlotInfo(uint16_t nSlotOffset, lightset::SlotInfo &tSlotInfo) override;

	void Display(const uint8_t *pData) __attribute__((weak));

private:
	static PixelDmxStore *s_pWS28xxDmxStore;
	static uint8_t s_Data;
};

#endif /* PIXELDMXPARAMSRDM_H_ */
