/**
 * @file mcp9808.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef MCP9808_H_
#define MCP9808_H_

#include <cstdint>

#include "hal_i2c.h"

namespace sensor {
namespace mcp9808 {
static constexpr char DESCRIPTION[] = "Ambient Temperature";
static constexpr auto RANGE_MIN = -20;
static constexpr auto RANGE_MAX = 100;
}  // namespace mcp9808

class MCP9808: HAL_I2C {
public:
	MCP9808(uint8_t nAddress = 0);

	bool Initialize() {
		return m_bIsInitialized;
	}

	float Get();

private:
	bool m_bIsInitialized { false };
};

}  // namespace sensor

#endif /* MCP9808_H_ */
