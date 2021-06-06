/**
 * @file mcp48x2.h
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef MCP48X2_H_
#define MCP48X2_H_

#include <cstdint>

#include "hal_spi.h"

/*
 * MCP4802: Dual 8-Bit Voltage Output DAC
 * MCP4812: Dual 10-Bit Voltage Output DAC
 * MCP4822: Dual 12-Bit Voltage Output DAC
 */

namespace dac {
namespace mcp48x2 {
namespace speed {
static constexpr uint32_t max_hz = 20000000;	///< 20 MHz
static constexpr uint32_t default_hz = 10000000;///< 10 MHz
}  // namespace speed
namespace mask {
static constexpr uint16_t dac_selection = (1U << 15);
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
}  // namespace mcp48x2

class MCP4822: HAL_SPI {
	uint16_t Data12Bit(const uint16_t nData) {
		return (nData & mcp48x2::mask::data_12bit) << mcp48x2::shift::data_12bit;
	}

public:
	MCP4822(uint8_t nChipSelect = 0, uint32_t nSpeedHz = mcp48x2::speed::default_hz) :
			HAL_SPI(nChipSelect, nSpeedHz == 0 ? mcp48x2::speed::default_hz :
							( mcp48x2::speed::default_hz <= mcp48x2::speed::max_hz ? nSpeedHz : mcp48x2::speed::max_hz) ) {
	}

	void WriteDacA(uint16_t nData) {
		const uint16_t nSpiData = mcp48x2::reg::write_dac_a
				| mcp48x2::reg::gain_1x | mcp48x2::reg::shutdown_off
				| Data12Bit(nData);
		HAL_SPI::Write(nSpiData);
	}

	void WriteDacB(uint16_t nData) {
		const uint16_t nSpiData = mcp48x2::reg::write_dac_b
				| mcp48x2::reg::gain_1x | mcp48x2::reg::shutdown_off
				| Data12Bit(nData);
		HAL_SPI::Write(nSpiData);
	}

	void WriteDacAB(uint16_t nDataA, uint16_t nDataB) {
		const uint16_t nSpiDataA = mcp48x2::reg::write_dac_a
				| mcp48x2::reg::gain_1x | mcp48x2::reg::shutdown_off
				| Data12Bit(nDataA);
		HAL_SPI::Write (nSpiDataA);

		const uint16_t nSpiDataB = mcp48x2::reg::write_dac_b
				| mcp48x2::reg::gain_1x | mcp48x2::reg::shutdown_off
				| Data12Bit(nDataB);
		HAL_SPI::Write(nSpiDataB, false);
	}
};

}  // namespace dac

#endif /* MCP48X2_H_ */
