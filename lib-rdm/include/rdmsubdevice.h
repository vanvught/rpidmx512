/**
 * @file rdmsubdevice.h
 *
 */
/* Copyright (C) 2018-2023 by Arjan van Vught mailto:info@gd32-dmx.org
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
	uint16_t dmx_start_address;
	uint8_t current_personality;
	uint8_t personality_count;
	char label[RDM_DEVICE_LABEL_MAX_LENGTH];
	uint8_t label_length;
	uint8_t sensor_count;
};

enum TRDMSubDeviceUpdateEvent {
	RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS,
	RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY
};

class RDMSubDevice {
public:
	explicit RDMSubDevice(const char* label, uint16_t dmx_start_address = 1, uint8_t personality_current = 1):
		dmx_start_address_factory_default_(dmx_start_address),
		current_personality_factory_default_(personality_current)
	{
		for (uint32_t i = 0; (i < RDM_DEVICE_LABEL_MAX_LENGTH) && label[i] != 0; i++) {
			label_factory_default_[i] = label[i];
		}

		SetLabel(label);

		sub_devices_info_.dmx_start_address = dmx_start_address;
		sub_devices_info_.current_personality = personality_current;

		sub_devices_info_.sensor_count = 0;
		sub_devices_info_.personality_count = 0;

		checksum_ = CalculateChecksum();
	}

	virtual ~RDMSubDevice() = default;

	void SetDmxStartAddress(uint16_t dmx_start_address) {
		sub_devices_info_.dmx_start_address = dmx_start_address;
		UpdateEvent(RDM_SUBDEVICE_UPDATE_EVENT_DMX_STARTADDRESS);
	}

	uint16_t GetDmxStartAddress() const {
		return sub_devices_info_.dmx_start_address;
	}

	uint8_t GetPersonalityCurrent() const {
		return sub_devices_info_.current_personality;
	}

	void SetPersonalityCurrent(uint8_t current) {
		sub_devices_info_.current_personality = current;
	}

	void GetLabel(struct rdm::DeviceInfoData* info_data) {
		assert(info_data != nullptr);

		info_data->data = sub_devices_info_.label;
		info_data->length = sub_devices_info_.label_length;
	}

	void SetLabel(const char *label) {
		assert(label != nullptr);
		uint32_t i;

		for (i = 0; (i < RDM_DEVICE_LABEL_MAX_LENGTH) && (label[i] != 0); i++) {
			sub_devices_info_.label[i] = label[i];
		}

		sub_devices_info_.label_length = static_cast<uint8_t>(i);
	}

	void SetLabel(const char *label, uint8_t label_length) {
		assert(label != nullptr);
		uint32_t i;

		for (i = 0; (i < RDM_DEVICE_LABEL_MAX_LENGTH) && (i < label_length); i++) {
			sub_devices_info_.label[i] = label[i];
		}

		sub_devices_info_.label_length = static_cast<uint8_t>(i);
	}

	struct TRDMSubDevicesInfo* GetInfo() {
		return &sub_devices_info_;
	}

	RDMPersonality *GetPersonality(uint8_t personality) {
		assert(personality != 0);
		assert(personality <= sub_devices_info_.personality_count);

		return personalities_[personality - 1];
	}

	uint8_t GetPersonalityCount() const {
		return sub_devices_info_.personality_count;
	}

	uint16_t GetDmxFootPrint() const {
		return sub_devices_info_.dmx_footprint;
	}

	bool GetFactoryDefaults() {
		if (is_factory_defaults_) {
			if (checksum_ != CalculateChecksum()) {
				is_factory_defaults_ = false;
			}
		}

		return is_factory_defaults_;
	}

	void SetFactoryDefaults() {
		if (is_factory_defaults_) {
			return;
		}

		SetLabel(label_factory_default_);

		sub_devices_info_.dmx_start_address = dmx_start_address_factory_default_;
		sub_devices_info_.current_personality = current_personality_factory_default_;

		is_factory_defaults_ = true;
	}

	virtual bool Initialize()=0;
	virtual void Start()= 0;
	virtual void Stop()= 0;
	virtual void Data(const uint8_t *data, uint32_t length)=0;

protected:
	void SetDmxFootprint(uint16_t dmx_footprint) {
		sub_devices_info_.dmx_footprint = dmx_footprint;
	}

	void SetPersonalities(RDMPersonality **personalities, uint8_t personality_count) {
		assert(personalities != nullptr);

		sub_devices_info_.personality_count = personality_count;
		personalities_ = personalities;

		UpdateEvent(RDM_SUBDEVICE_UPDATE_EVENT_PERSONALITY);
	}

private:
	virtual void UpdateEvent([[maybe_unused]] TRDMSubDeviceUpdateEvent update_event) {}
	uint16_t CalculateChecksum() {
		uint16_t checksum = sub_devices_info_.dmx_start_address;
		checksum = static_cast<uint16_t>(checksum + sub_devices_info_.current_personality);

		for (uint32_t i = 0; i < sub_devices_info_.label_length; i++) {
			checksum = static_cast<uint16_t>(checksum + sub_devices_info_.label[i]);
		}

		return checksum;
	}

private:
 RDMPersonality** personalities_{nullptr};
 bool is_factory_defaults_{true};
 uint16_t checksum_{0};
 uint16_t dmx_start_address_factory_default_;
 uint8_t current_personality_factory_default_;
 TRDMSubDevicesInfo sub_devices_info_;
 char label_factory_default_[RDM_DEVICE_LABEL_MAX_LENGTH];
};

#endif  // RDMSUBDEVICE_H_
