/**
 * @file rdmsubdevices.h
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

#ifndef RDMSUBDEVICES_H_
#define RDMSUBDEVICES_H_

#include <stdint.h>

#include "rdmsubdevice.h"
#include "rdmsubdevicesconst.h"

#include "rdmpersonality.h"

#if defined(BARE_METAL) && !( defined(ARTNET_NODE) && defined(RDM_RESPONDER) )
# define RDM_SUBDEVICES_ENABLE
#endif

#if defined (RDMNET_LLRP_ONLY)
# undef RDM_SUBDEVICES_ENABLE
#endif

namespace rdm {
namespace subdevices {
static constexpr uint32_t max = 8;
static constexpr uint32_t store = 96;	// bytes
}  // namespace subdevices
}  // namespace rdm

class RDMSubDevices {
public:
	RDMSubDevices();
	~RDMSubDevices();

	bool Add(RDMSubDevice *pRDMSubDevice);
	uint16_t GetCount() {
		return m_nCount;
	}

	struct TRDMSubDevicesInfo *GetInfo(uint16_t nSubDevice);

	uint16_t GetDmxFootPrint(uint16_t nSubDevice);

	RDMPersonality* GetPersonality(uint16_t nSubDevice , uint8_t nPersonality);
	uint8_t GetPersonalityCount(uint16_t nSubDevice);
	uint8_t GetPersonalityCurrent(uint16_t nSubDevice);
	void SetPersonalityCurrent(uint16_t nSubDevice, uint8_t nPersonality);

	// E120_DEVICE_LABEL			0x0082
	void GetLabel(uint16_t nSubDevice, struct TRDMDeviceInfoData *pInfoData);
	void SetLabel(uint16_t nSubDevice, const char *pLabel, uint8_t nLabelLength);

	// E120_FACTORY_DEFAULTS		0x0090
	bool GetFactoryDefaults();
	void SetFactoryDefaults();

	// E120_DMX_START_ADDRESS		0x00F0
	uint16_t GetDmxStartAddress(uint16_t nSubDevice);
	void SetDmxStartAddress(uint16_t nSubDevice, uint16_t nDmxStartAddress);

	void Start();
	void Stop();
	void SetData(const uint8_t *pData, uint16_t nLength);

	static const char *GetTypeString(rdm::subdevices::type tType);
	static rdm::subdevices::type GetTypeString(const char *pValue);

	static RDMSubDevices* Get() {
		return s_pThis;
	}

private:
	RDMSubDevice **m_pRDMSubDevice;
	uint16_t m_nCount{0};

	static RDMSubDevices *s_pThis;
};

#endif /* RDMSUBDEVICES_H_ */
