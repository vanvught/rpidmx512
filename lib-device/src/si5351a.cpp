/**
 * @file si5351a.cpp
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


#include <cstdint>
#include <cassert>

#include "si5351a.h"
#include "hal_i2c.h"

#include "debug.h"

namespace si5351a {
static constexpr uint8_t I2C_ADDRESS = 0x60;
namespace clock_builder {
static constexpr auto REGS = 57;
}  // namespace clock_builder
}  // namespace si5351a

struct TRegister {
	uint8_t nAddress;
	uint8_t nValue;
};

TRegister static constexpr registers[si5351a::clock_builder::REGS] =
{
	{ 0x0002, 0x53 },
	{ 0x0003, 0x00 },
	{ 0x0007, 0x00 },
	{ 0x000F, 0x00 },
	{ 0x0010, 0x0F },
	{ 0x0011, 0x0F },
	{ 0x0012, 0x0F },
	{ 0x0013, 0x8C },
	{ 0x0014, 0x8C },
	{ 0x0015, 0x8C },
	{ 0x0016, 0x8C },
	{ 0x0017, 0x8C },
	{ 0x001A, 0x00 },
	{ 0x001B, 0x7D },
	{ 0x001C, 0x00 },
	{ 0x001D, 0x0F },
	{ 0x001E, 0xFB },
	{ 0x001F, 0x00 },
	{ 0x0020, 0x00 },
	{ 0x0021, 0x71 },
	{ 0x002A, 0x00 },
	{ 0x002B, 0x01 },
	{ 0x002C, 0x12 },
	{ 0x002D, 0x30 },
	{ 0x002E, 0x00 },
	{ 0x002F, 0x00 },
	{ 0x0030, 0x00 },
	{ 0x0031, 0x00 },
	{ 0x0032, 0x00 },
	{ 0x0033, 0x01 },
	{ 0x0034, 0x02 },
	{ 0x0035, 0x30 },
	{ 0x0036, 0x00 },
	{ 0x0037, 0x00 },
	{ 0x0038, 0x00 },
	{ 0x0039, 0x00 },
	{ 0x003A, 0x00 },
	{ 0x003B, 0x02 },
	{ 0x003C, 0x00 },
	{ 0x003D, 0x44 },
	{ 0x003E, 0x40 },
	{ 0x003F, 0x00 },
	{ 0x0040, 0x00 },
	{ 0x0041, 0x00 },
	{ 0x005A, 0x00 },
	{ 0x005B, 0x00 },
	{ 0x0095, 0x00 },
	{ 0x0096, 0x00 },
	{ 0x0097, 0x00 },
	{ 0x0098, 0x00 },
	{ 0x0099, 0x00 },
	{ 0x009A, 0x00 },
	{ 0x009B, 0x00 },
	{ 0x00A2, 0x00 },
	{ 0x00A3, 0x00 },
	{ 0x00A4, 0x00 },
	{ 0x00B7, 0xD2 },
};

using namespace si5351a;

SI5351A::SI5351A(uint8_t nAddress) : HAL_I2C(nAddress == 0 ? I2C_ADDRESS : nAddress) {
	DEBUG_ENTRY

	m_bIsConnected = HAL_I2C::IsConnected();

	DEBUG_PRINTF("m_bIsConnected=%d", m_bIsConnected);
	DEBUG_EXIT
}

void SI5351A::ClockBuilder() {
	DEBUG_ENTRY
	assert(m_bIsConnected);

	Pre();

	for (uint32_t i = 0; i < clock_builder::REGS; i++) {
		HAL_I2C::WriteRegister(registers[i].nAddress, registers[i].nValue);
	}

	Post();

	DEBUG_EXIT
}

void SI5351A::Pre() {
	/*
	 * Disable Outputs. Set CLKx_DIS high; Reg. 3 = 0xFF
	 */
	HAL_I2C::WriteRegister(3, static_cast<uint8_t>(0xFF));

	/*
	 * Powerdown all output drivers Reg. 16, 17, 18, 19, 20, 21, 22, 23 = 0x80
	 */
	for (uint32_t i = 16 ; i <= 23; i++) {
		HAL_I2C::WriteRegister(static_cast<uint8_t>(i), static_cast<uint8_t>(0x80));
	}
}

void SI5351A::Post() {
	/**
	 * Apply PLLA and PLLB soft reset Reg. 177 = 0xAC
	 */
	HAL_I2C::WriteRegister(177, static_cast<uint8_t>(0xAC));

	/*
	 * Enable outputs
	 */
	HAL_I2C::WriteRegister(3, static_cast<uint8_t>(0x00));
}
