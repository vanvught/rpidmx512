/**
 * @file rdmdevice.h
 *
 */
/* Copyright (C) 2017-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef RDMDEVICE_H_
#define RDMDEVICE_H_

#include <stdint.h>
#include <stdbool.h>

enum TRDMDeviceMask {
	RDMDEVICE_MASK_MANUFACTURER_NAME = (1 << 0),
	RDMDEVICE_MASK_MANUFACTURER_ID = (1 << 1),
	RDMDEVICE_MASK_LABEL = (1 << 2),
	RDMDEVICE_MASK_EXTERNAL_MONITOR = (1 << 3),
	RDMDEVICE_MASK_PRODUCT_CATEGORY = (1 << 4),
	RDMDEVICE_MASK_PRODUCT_DETAIL = (1 << 5)
};

struct TRDMDeviceInfoData {
	/*@shared@*//*@null@*/uint8_t *data;
	uint8_t length;
};

///< http://rdm.openlighting.org/pid/display?manufacturer=0&pid=96
struct TRDMDeviceInfo {
	uint8_t protocol_major;			///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
	uint8_t protocol_minor;			///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
	uint8_t device_model[2];		///< This field identifies the Device Model ID of the Root Device or the Sub-Device. The Manufacturer shall not use the same ID to represent more than one unique model type.
	uint8_t product_category[2];	///< Devices shall report a Product Category based on the products primary function.
	uint8_t software_version[4];	///< This field indicates the Software Version ID for the device. The Software Version ID is a 32-bit value determined by the Manufacturer.
	uint8_t dmx_footprint[2];		///< If the DEVICE_INFO message is directed to a Sub-Device, then the response for this field contains the DMX512 Footprint for that Sub-Device. If the message is sent to the Root Device, it is the Footprint for the Root Device itself. If the Device or Sub-Device does not utilize Null Start Code packets for any control or functionality then it shall report a Footprint of 0x0000.
	uint8_t current_personality;	///<
	uint8_t personality_count;		///<
	uint8_t dmx_start_address[2];	///< If the Device or Sub-Device that this message is directed to has a DMX512 Footprint of 0, then this field shall be set to 0xFFFF.
	uint8_t sub_device_count[2];	///< The response for this field shall always be same regardless of whether this message is directed to the Root Device or a Sub-Device.
	uint8_t sensor_count;			///< This field indicates the number of available sensors in a Root Device or Sub-Device. When this parameter is directed to a Sub-Device, the reply shall be identical for any Sub-Device owned by a specific Root Device.
};

#include "rdm.h"

#define DEVICE_SN_LENGTH		4

struct TRDMDeviceParams {
    uint32_t nSetList;
	uint8_t aDeviceUID[RDM_UID_SIZE];
	uint8_t aDeviceSN[DEVICE_SN_LENGTH];
	char aDeviceRootLabel[RDM_DEVICE_LABEL_MAX_LENGTH];
	uint8_t nDeviceRootLabelLength;
	char aDeviceManufacturerName[RDM_MANUFACTURER_LABEL_MAX_LENGTH];
	uint8_t nDdeviceManufacturerNameLength;
	uint16_t nProductCategory;
	uint16_t nProductDetail;
	uint8_t nExtMonLevel;
};

class RDMDeviceStore {
public:
	virtual ~RDMDeviceStore(void);

	virtual void Update(const struct TRDMDeviceParams *pRDMDeviceParams)=0;
	virtual void Copy(struct TRDMDeviceParams *pRDMDeviceParams)=0;

	virtual void SaveLabel(const uint8_t* pLabel, uint8_t nLength)=0;
};

class RDMDevice {
public:
	RDMDevice(const struct TRDMDeviceInfoData *tLabel = 0, RDMDeviceStore* m_pRDMDeviceParamsStore = 0);
	~RDMDevice(void);

	bool Load(void);
	void Load(const char *pBuffer, uint32_t nLength);

	bool Save(uint8_t *pBuffer, uint32_t nLength, uint32_t& nSize);

	void Dump(void);

	void SetLabel(const struct TRDMDeviceInfoData *);
	void GetLabel(struct TRDMDeviceInfoData *);

	void GetManufacturerId(struct TRDMDeviceInfoData *);

	void GetManufacturerName(struct TRDMDeviceInfoData *);

	char *GetSoftwareVersion(void);
	uint8_t GetSoftwareVersionLength(void) const;

	uint32_t GetSoftwareVersionId(void) const;

	const uint8_t* GetUID(void) {
		return (const uint8_t *) m_tRDMDeviceParams.aDeviceUID;
	}

	const uint8_t* GetSN(void) {
		return (const uint8_t *) m_tRDMDeviceParams.aDeviceSN;
	}

	uint16_t GetProductCategory(void) {
		return m_tRDMDeviceParams.nProductCategory;
	}

	uint16_t GetProductDetail(void) {
		return m_tRDMDeviceParams.nProductDetail;
	}

	uint8_t GetExtMonLevel(void) {
		return m_tRDMDeviceParams.nExtMonLevel;
	}

public:
    static void staticCallbackFunction(void *p, const char *s);

private:
    void callbackFunction(const char *s);
	bool isMaskSet(uint32_t nMask);

private:
	RDMDeviceStore* m_pRDMDeviceParamsStore;
	struct TRDMDeviceParams m_tRDMDeviceParams;
};

#endif /* RDMDEVICE_H_ */
