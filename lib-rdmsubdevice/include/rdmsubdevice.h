/**
 * @file rdmsubdevice.h
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

#ifndef RDMSUBDEVICE_H_
#define RDMSUBDEVICE_H_

#include "rdmdevice.h"
#include "rdmpersonality.h"

struct TRDMSubDevicesInfo {
	uint16_t dmx_footprint;
	uint8_t current_personality;
	uint8_t personality_count;
	uint16_t dmx_start_address;
	uint8_t sensor_count;
	char device_label[RDM_DEVICE_LABEL_MAX_LENGTH];
	uint8_t device_label_length;
};

enum TRDMSubDeviceUpdateEvent {
	RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS,
	RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY
};

class RDMSubDevice {
public:
	RDMSubDevice(const char* pLabel, uint16_t nDmxStartAddress = 1, uint8_t PersonalitynCurrent = 1);
	virtual ~RDMSubDevice(void);

	inline uint16_t GetDmxStartAddress(void) { return m_tSubDevicesInfo.dmx_start_address;}
	void SetDmxStartAddress(uint16_t nDmxStartAddress);

	inline uint8_t GetPersonalityCurrent(void) { return m_tSubDevicesInfo.current_personality;}
	void SetPersonalityCurrent(uint8_t nCurrent);

	void GetLabel(struct TRDMDeviceInfoData *pInfoData);
	void SetLabel(const char* pLabel);
	void SetLabel(const char* pLabel, uint8_t nLabelLength);

	inline struct TRDMSubDevicesInfo *GetInfo(void) { return &m_tSubDevicesInfo; }

	RDMPersonality* GetPersonality(uint8_t nPersonality);

	inline uint8_t GetPersonalityCount(void) { return m_tSubDevicesInfo.personality_count;}

	inline uint16_t GetDmxFootPrint(void) { return m_tSubDevicesInfo.dmx_footprint;}

	bool GetFactoryDefaults(void);
	void SetFactoryDefaults(void);

	virtual bool Initialize(void)=0;

	virtual void Start(void)= 0;
	virtual void Stop(void)= 0;
	virtual void Data(const uint8_t *pDdata, uint16_t nLength)=0;

	virtual void UpdateEvent(TRDMSubDeviceUpdateEvent tUpdateEvent)=0;

protected:
	void SetDmxFootprint(uint16_t nDmxFootprint);
	void SetPersonalities(RDMPersonality **pRDMPersonalities, uint8_t nPersonalityCount);

private:
	uint16_t CalculateChecksum(void);

private:
	RDMPersonality **m_pRDMPersonalities;
	bool m_IsFactoryDefaults;
	uint16_t m_nCheckSum;
	char m_aLabelFactoryDefault[RDM_DEVICE_LABEL_MAX_LENGTH];
	uint16_t m_nDmxStartAddressFactoryDefault;
	uint8_t m_nCurrentPersonalityFactoryDefault;
	struct TRDMSubDevicesInfo m_tSubDevicesInfo;
};

#endif /* RDMSUBDEVICE_H_ */
