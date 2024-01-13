/**
 * @file rdmsubdevicebwdio.h
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

#ifndef RDMSUBDEVICEBWDIO_H_
#define RDMSUBDEVICEBWDIO_H_

#include <cstdint>

#include "rdmsubdevice.h"

#include "bwspidio.h"

class RDMSubDeviceBwDio: public RDMSubDevice {
public:
	RDMSubDeviceBwDio(uint16_t nDmxStartAddress = 1, char nChipSselect = 0, uint8_t nSlaveAddress = bw::dio::address, uint32_t nSpiSpeed = bw::spi::speed::default_hz);

	bool Initialize() override {
		if (m_BwSpiDio.IsConnected()) {
			m_BwSpiDio.SetDirection(0x7F);
			m_BwSpiDio.Output(0x00);
			return true;
		}
		return false;
	}

	void Start() override {
	}

	void Stop() override {
		m_BwSpiDio.Output(0x00);
		m_nData = 0;
	}

	void Data(const uint8_t *pData, uint32_t nLength) override;

private:
	void UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent) override;

private:
	BwSpiDio m_BwSpiDio;
	uint8_t m_nData = 0;
};

#endif /* RDMSUBDEVICEBWDIO_H_ */
