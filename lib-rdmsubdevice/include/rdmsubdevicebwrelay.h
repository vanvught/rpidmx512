/**
 * @file rdmsubdevicebwrelay.h
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef RDMSUBDEVICEBWRELAY_H_
#define RDMSUBDEVICEBWRELAY_H_

#include <stdint.h>

#include "rdmsubdevice.h"

#include "bw_spi_relay.h"

class RDMSubDeviceBwRelay: public RDMSubDevice {
public:
	RDMSubDeviceBwRelay(uint16_t nDmxStartAddress = 1, char nChipSselect = 0, uint8_t nSlaveAddress = 0, uint32_t nSpiSpeed = 0);
	~RDMSubDeviceBwRelay(void);

	bool Initialize(void);

	void Start(void);
	void Stop(void);
	void Data(const uint8_t *pData, uint16_t nLength);

	void UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent);

private:
	struct _device_info m_tDeviceInfo;
	uint8_t m_nData;
};

#endif /* RDMSUBDEVICEBWRELAY_H_ */
