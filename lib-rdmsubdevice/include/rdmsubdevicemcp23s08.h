/**
 * @file rdmsubdevicemcp23s08.h
 *
 */
/* Copyright (C) 2018-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef RDMSUBDEVICEMCP23S08_H_
#define RDMSUBDEVICEMCP23S08_H_

#include <cstdint>

#include "rdmsubdevice.h"

#include "mcp23s08.h"

class RDMSubDeviceMCP23S08: public RDMSubDevice {
public:
	RDMSubDeviceMCP23S08(uint16_t nDmxStartAddress = 1, char nChipSselect = 0, uint8_t nSlaveAddress = 0, uint32_t nSpiSpeed = 0);

	bool Initialize() override {
		m_MCP23S08.WriteRegister(gpio::mcp23s08::reg::IODIR, 0x00);
		m_MCP23S08.WriteRegister(gpio::mcp23s08::reg::GPIO, 0x00);
		return true;
	}

	void Start() override {
	}

	void Stop() override {
		m_MCP23S08.WriteRegister(gpio::mcp23s08::reg::GPIO, 0x00);
		m_nData = 0;
	}

	void Data(const uint8_t *pData, uint32_t nLength) override;

private:
	void UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) override;

private:
	gpio::MCP23S08 m_MCP23S08;
	uint8_t m_nData = 0;
};

#endif /* RDMSUBDEVICEMCP23S08_H_ */
