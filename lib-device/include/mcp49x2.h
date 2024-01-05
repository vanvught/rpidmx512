/**
 * @file mcp49x2.h
 *
 */
/* Copyright (C) 2014-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef MCP49X2_H_
#define MCP49X2_H_

#include <cstdint>

#include "hal_spi.h"

/*
 * MCP4902: Dual 8-Bit Voltage Output DAC
 * MCP4912: Dual 10-Bit Voltage Output DAC
 * MCP4922: Dual 12-Bit Voltage Output DAC
 */

namespace dac {
namespace mcp49x2 {
namespace speed {
static constexpr uint32_t max_hz = 20000000;	///< 20 MHz
static constexpr uint32_t default_hz = 10000000;///< 10 MHz
}  // namespace speed
namespace mask {
static constexpr uint16_t dac_selection = (1U << 15);
static constexpr uint16_t buffer = (1U << 14);
static constexpr uint16_t gain = (1U << 13);
static constexpr uint16_t shutdown = (1U << 12);
static constexpr uint16_t data_12bit = 0x0FFF;
static constexpr uint16_t data_10bit = 0x03FF;
static constexpr uint16_t data_8bit = 0x00FF;
}  // namespace mask
namespace shift {
static constexpr uint16_t data_12bit = 0;
static constexpr uint16_t data_10bit = 2;
static constexpr uint16_t data_8bit = 4;
}  // namespace shift
namespace reg {
static constexpr uint16_t write_dac_a = (0 << 15);
static constexpr uint16_t write_dac_b = (1U << 15);
static constexpr uint16_t gain_2x = (0 << 13);
static constexpr uint16_t gain_1x = (1U << 13);
static constexpr uint16_t shutdown_on = (0 << 12);
static constexpr uint16_t shutdown_off = (1U << 12);
}  // namespace reg
}  // namespace mcp49x2

class MCP4902: HAL_SPI {
	uint16_t Data8Bit(const uint8_t nData) {
		return static_cast<uint16_t>((nData & mcp49x2::mask::data_8bit) << mcp49x2::shift::data_8bit);
	}

public:
	MCP4902(uint8_t nChipSelect = 0, uint32_t nSpeedHz = mcp49x2::speed::default_hz) :
			HAL_SPI(nChipSelect, nSpeedHz == 0 ? mcp49x2::speed::default_hz :
							( mcp49x2::speed::default_hz <= mcp49x2::speed::max_hz ? nSpeedHz : mcp49x2::speed::max_hz) ) {
	}

	void WriteDacA(uint8_t nData) {
		const uint16_t nSpiData = mcp49x2::reg::write_dac_a
				| mcp49x2::reg::gain_1x | mcp49x2::reg::shutdown_off
				| Data8Bit(nData);
		HAL_SPI::Write(nSpiData);
	}

	void WriteDacB(uint8_t nData) {
		const uint16_t nSpiData = mcp49x2::reg::write_dac_b
				| mcp49x2::reg::gain_1x | mcp49x2::reg::shutdown_off
				| Data8Bit(nData);
		HAL_SPI::Write(nSpiData);
	}

	void WriteDacAB(uint8_t nDataA, uint8_t nDataB) {
		const uint16_t nSpiDataA = mcp49x2::reg::write_dac_a
				| mcp49x2::reg::gain_1x | mcp49x2::reg::shutdown_off
				| Data8Bit(nDataA);
		HAL_SPI::Write (nSpiDataA);

		const uint16_t nSpiDataB = mcp49x2::reg::write_dac_b
				| mcp49x2::reg::gain_1x | mcp49x2::reg::shutdown_off
				| Data8Bit(nDataB);
		HAL_SPI::Write(nSpiDataB, false);
	}
};

}  // namespace dac

#endif /* MCP49X2_H_ */
