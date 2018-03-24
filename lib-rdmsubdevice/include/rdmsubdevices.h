/**
 * @file rdmsubdevices.h
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

#ifndef RDMSUBDEVICES_H_
#define RDMSUBDEVICES_H_

#include <stdint.h>

#include "rdmsubdevice.h"
#include "rdmpersonality.h"

class RDMSubDevices {
public:
	RDMSubDevices(void);
	~RDMSubDevices(void);

	void Init(void);
	bool Add(RDMSubDevice *pRDMSubDevice);
	uint16_t GetCount(void) const;

	struct TRDMSubDevicesInfo *GetInfo(uint16_t nSubDevice);

	uint16_t GetDmxFootPrint(uint16_t nSubDevice);

	RDMPersonality* GetPersonality(uint16_t nSubDevice , uint8_t nPersonality);
	uint8_t GetPersonalityCount(uint16_t nSubDevice);
	uint8_t GetPersonalityCurrent(uint16_t nSubDevice);
	void SetPersonalityCurrent(uint16_t nSubDevice, uint8_t nPersonality);

	// E120_DEVICE_LABEL			0x0082
	void GetLabel(uint16_t nSubDevice, struct TRDMDeviceInfoData *pInfoData);
	void SetLabel(uint16_t nSubDevice, const uint8_t *pLabel, uint8_t nLabelLength);

	// E120_FACTORY_DEFAULTS		0x0090
	bool GetFactoryDefaults(void);
	void SetFactoryDefaults(void);

	// E120_DMX_START_ADDRESS		0x00F0
	uint16_t GetDmxStartAddress(uint16_t nSubDevice);
	void SetDmxStartAddress(uint16_t nSubDevice, uint16_t nDmxStartAddress);

public:
	void Start(void);
	void Stop(void);
	void SetData(const uint8_t *pData, uint16_t nLength);

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);

private:
	RDMSubDevice **m_pRDMSubDevice;
	uint16_t m_nCount;
};

#endif /* RDMSUBDEVICES_H_ */
