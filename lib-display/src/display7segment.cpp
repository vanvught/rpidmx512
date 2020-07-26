/**
 * @file display7segment.cpp
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

#include <stdint.h>
#include <assert.h>

#include "display7segment.h"

#include "hal_i2c.h"

#define MCP23017_I2C_ADDRESS	0x20
#define SEGMENT7_I2C_ADDRESS	(MCP23017_I2C_ADDRESS + 1)	///< It must be different from base address
#define MCP23X17_IODIRA			0x00	///< I/O DIRECTION (IODIRA) REGISTER, 1 = Input (default), 0 = Output
#define MCP23X17_GPIOA			0x12	///< PORT (GPIOA) REGISTER, Value on the Port - Writing Sets Bits in the Output Latch

Display7Segment *Display7Segment::s_pThis = nullptr;

Display7Segment::Display7Segment() : m_I2C(SEGMENT7_I2C_ADDRESS) {
	assert(s_pThis == nullptr);
	s_pThis = this;

	m_bHave7Segment = m_I2C.IsConnected();

	if (m_bHave7Segment) {
		m_I2C.WriteRegister(MCP23X17_IODIRA, static_cast<uint16_t>(0x0000)); // All output
		Status(Display7SegmentMessage::INFO_STARTUP);
	}
}

void Display7Segment::Status(Display7SegmentMessage nData) {
	if (m_bHave7Segment) {
		m_I2C.WriteRegister(MCP23X17_GPIOA, static_cast<uint16_t>(~static_cast<uint16_t>(nData)));
	}
}

void Display7Segment::Status(uint8_t nValue, bool bHex) {
	if (m_bHave7Segment) {
		uint16_t nData;

		if (!bHex) {
			nData = GetData(nValue / 10);
			nData |= GetData(nValue % 10) << 8;
		} else {
			nData = GetData(nValue & 0x0F);
			nData |= GetData((nValue >> 4) & 0x0F) << 8;
		}

		m_I2C.WriteRegister(MCP23X17_GPIOA, static_cast<uint16_t>(~nData));
	}
}
