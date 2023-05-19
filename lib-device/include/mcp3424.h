/**
 * @file mcp3424.h
 *
 */
/* Copyright (C) 2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef MCP3424_H_
#define MCP3424_H_

#include <cstdint>

#include "hal_i2c.h"

namespace adc {
namespace mcp3424 {
enum class Gain {
	PGA_X1,			///< Default
	PGA_X2,
	PGA_X4,
	PGA_X8,
};

enum class Resolution {
	SAMPLE_12BITS,	///< Default
	SAMPLE_14BITS,
	SAMPLE_16BITS,
	SAMPLE_18BITS
};

enum class Conversion {
	ONE_SHOT,
	CONTINUOUS		///< Default
};
}  // namespace mcp3424
}  // namespace adc

class MCP3424: HAL_I2C {
public:
	MCP3424(uint8_t nAddress = 0);

	bool IsConnected() {
		return m_IsConnected;
	}

	void SetGain(const adc::mcp3424::Gain gain);
	adc::mcp3424::Gain GetGain() const;

	void SetResolution(const adc::mcp3424::Resolution resolution);
	adc::mcp3424::Resolution GetResolution() const;

	void SetConversion(const adc::mcp3424::Conversion conversion);
	adc::mcp3424::Conversion GetConversion() const;

	uint32_t GetRaw(const uint32_t nChannel);
	double GetVoltage(const uint32_t nChannel);

private:
	bool m_IsConnected { false };
	uint8_t m_nConfig;
	double m_lsb;
};

#endif /* IMCP3424_H_ */
