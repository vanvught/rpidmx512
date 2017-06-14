/**
 * @file rdm_device_info.c
 *
 */
/* Copyright (C) 2015, 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>

#include "read_config_file.h"
#include "hardware.h"
#include "util.h"
#include "sscan.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_device_info.h"
#include "rdm_device_const.h"
#if defined(RDM_RESPONDER)
#include "rdm_e120.h"
#include "rdm_sub_devices.h"
#include "rdm_sensor.h"
#include "rdm_identify.h"
#endif

static const uint8_t DEVICE_LABEL_LENGTH = sizeof(DEVICE_LABEL) / sizeof(DEVICE_LABEL[0]) - 1;
static const uint8_t DEVICE_MANUFACTURER_NAME_LENGTH = sizeof(DEVICE_MANUFACTURER_NAME) / sizeof(DEVICE_MANUFACTURER_NAME[0]) - 1;
#ifdef RDM_RESPONDER
static const uint8_t DEVICE_SOFTWARE_VERSION_LENGTH = sizeof(DEVICE_SOFTWARE_VERSION) / sizeof(DEVICE_SOFTWARE_VERSION[0]) - 1;
static const char RDM_DEVICE_PRODUCT_CATEGORY[] ALIGNED = "product_category";			///<
static const char RDM_DEVICE_PRODUCT_DETAIL[] ALIGNED = "product_detail";				///<
#endif

static const char RDM_DEVICE_FILE_NAME[] ALIGNED = "rdm_device.txt";					///< Parameters file name
static const char RDM_DEVICE_MANUFACTURER_NAME[] ALIGNED = "manufacturer_name";			///<
static const char RDM_DEVICE_MANUFACTURER_ID[] ALIGNED = "manufacturer_id";				///<
static const char RDM_DEVICE_LABEL[] ALIGNED = "device_label";							///<
static const char RDM_DEVICE_EXTERNAL_MONITOR[] ALIGNED = "device_external_monitor";	///<

// 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static uint8_t uid_device[RDM_UID_SIZE] = { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00 };	///<
static char root_device_label[RDM_DEVICE_LABEL_MAX_LENGTH] ALIGNED;					///<
static uint8_t root_device_label_length = (uint8_t) 0;								///<

static char device_manufacturer_name[RDM_MANUFACTURER_LABEL_MAX_LENGTH] ALIGNED;	///<
static uint8_t device_manufacturer_name_length = (uint8_t) 0;						///<

static uint8_t manufacturer_id[RDM_DEVICE_MANUFACTURER_ID_LENGTH] ALIGNED;			///<

static uint8_t device_sn[DEVICE_SN_LENGTH] ALIGNED;									///<

static uint8_t ext_mon_level = (uint8_t) 0;											///<

#ifdef RDM_RESPONDER
static uint16_t product_category = E120_PRODUCT_CATEGORY_OTHER;
static uint16_t product_detail = E120_PRODUCT_DETAIL_OTHER;

static bool is_factory_defaults = true;												///<
static uint16_t factory_defaults_checksum = (uint16_t) 0;							///<

static struct _rdm_device_info rdm_device_info ALIGNED;								///<
static struct _rdm_device_info rdm_sub_device_info ALIGNED;							///<
#endif

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t rdm_device_info_get_ext_mon_level(void) {
	return ext_mon_level;
}

/**
 * @ingroup rdm
 *
 * Process the input , parse name=value
 *
 * @param line
 */
static void process_line_read_string(const char *line) {
	char value[8] ALIGNED;
	uint8_t len;

	len = 1;
	if (sscan_char_p(line, RDM_DEVICE_EXTERNAL_MONITOR, value, &len) == 2) {
		if (len == 1) {
			if (isdigit((int)value[0])) {
				ext_mon_level = (uint8_t)(value[0] - (char)'0');
			}
		}
		return;
	}

	len = RDM_MANUFACTURER_LABEL_MAX_LENGTH;
	if (sscan_char_p(line, RDM_DEVICE_MANUFACTURER_NAME, device_manufacturer_name, &len) == 2) {
		device_manufacturer_name_length = len;
		return;
	}

	len = RDM_DEVICE_LABEL_MAX_LENGTH;
	if (sscan_char_p(line, RDM_DEVICE_LABEL, root_device_label, &len) == 2) {
		root_device_label_length = len;
		return;
	}

	len = 4;
	memset(value, 0, sizeof(value) / sizeof(char));
	if (sscan_char_p(line, RDM_DEVICE_MANUFACTURER_ID, value, &len) == 2) {
		if (len == 4) {
			const uint16_t v = (uint16_t) hex_uint32(value);
			uid_device[0] = (uint8_t) (v >> 8);
			uid_device[1] = (uint8_t) (v & 0xFF);
		}
		return;
	}

#ifdef RDM_RESPONDER
	len = 4;
	memset(value, 0, sizeof(value) / sizeof(char));
	if (sscan_char_p(line, RDM_DEVICE_PRODUCT_CATEGORY, value, &len) == 2) {
		if (len == 4) {
			product_category = (uint16_t) hex_uint32(value);
		}
		return;
	}

	len = 4;
	memset(value, 0, sizeof(value) / sizeof(char));
	if (sscan_char_p(line, RDM_DEVICE_PRODUCT_DETAIL, value, &len) == 2) {
		if (len == 4) {
			product_detail = (uint16_t) hex_uint32(value);
		}
		return;
	}
#endif
}

#ifdef RDM_RESPONDER
/**
 * @ingroup rdm
 *
 * Calculate the checksum over rdm_device_info.dmx_start_address, rdm_device_info.current_personality and root_device_label
 *
 * @return checksum
 */
inline static uint16_t calculate_checksum(void) {
	uint8_t i;
	uint16_t sub_device;
	uint8_t j;
	struct _rdm_device_info_data info;

	uint16_t checksum = (rdm_device_info.dmx_start_address[0] >> 8) + rdm_device_info.dmx_start_address[1];
	checksum += rdm_device_info.current_personality;

	for (i = 0; i < root_device_label_length; i++) {
		checksum += (uint16_t) root_device_label[i];
	}

	sub_device = (rdm_device_info.sub_device_count[0] << 8) + rdm_device_info.sub_device_count[1];

	for (i = 1; i <= sub_device; i++) {
		rdm_sub_devices_get_label(i, &info);
		for (j = 0; j < info.length; j++) {
			checksum += (uint16_t) info.data[i];
		}
	}

	return checksum;
}

/**
 *
 * @return
 */
const uint16_t rdm_device_info_get_product_category(void) {
	return product_category;
}

/**
 *
 * @return
 */
const uint16_t rdm_device_info_get_product_detail(void) {
	return product_detail;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const bool rdm_device_info_get_is_factory_defaults() {
	if (is_factory_defaults) {
		if (factory_defaults_checksum != calculate_checksum()) {
			is_factory_defaults = false;
		}
	}
	return is_factory_defaults;
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @param label
 * @param label_length
 */
void rdm_device_info_set_label(const uint16_t sub_device, const uint8_t *label,	uint8_t label_length) {
	if (label_length > RDM_DEVICE_LABEL_MAX_LENGTH) {
		label_length = RDM_DEVICE_LABEL_MAX_LENGTH;
	}

	if (sub_device != RDM_ROOT_DEVICE) {
		rdm_sub_devices_set_label(sub_device, label, label_length);
		return;
	}

	memcpy(root_device_label, label, label_length);
	root_device_label_length = label_length;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const char * rdm_device_info_get_supported_language(void) {
	return DEVICE_SUPPORTED_LANGUAGE;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t rdm_device_info_get_supported_language_length(void) {
	return RDM_DEVICE_SUPPORTED_LANGUAGE_LENGTH;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint32_t rdm_device_info_get_software_version_id(void) {
	return DEVICE_SOFTWARE_VERSION_ID;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const char * rdm_device_info_get_software_version(void) {
	return DEVICE_SOFTWARE_VERSION;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t rdm_device_info_get_software_version_length(void) {
	return DEVICE_SOFTWARE_VERSION_LENGTH;
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @return
 */
const uint16_t rdm_device_info_get_dmx_footprint(const uint16_t sub_device) {
	uint16_t dmx_footprint;

	if (sub_device != RDM_ROOT_DEVICE) {
		return rdm_sub_devices_get_footprint(sub_device);
	}

	dmx_footprint = (rdm_device_info.dmx_footprint[0] << 8) + rdm_device_info.dmx_footprint[1];

	return dmx_footprint;
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @return
 */
const uint16_t rdm_device_info_get_dmx_start_address(const uint16_t sub_device) {
	uint16_t dmx_start_address;

	if (sub_device != RDM_ROOT_DEVICE) {
		return rdm_sub_devices_get_dmx_start_address(sub_device);
	}

	dmx_start_address = (rdm_device_info.dmx_start_address[0] << 8) + rdm_device_info.dmx_start_address[1];

	return dmx_start_address;
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @param start_address
 */
void rdm_device_info_set_dmx_start_address(const uint16_t sub_device, const uint16_t start_address) {
	if (start_address == 0 || start_address > DMX_UNIVERSE_SIZE)
		return;

	if (sub_device != RDM_ROOT_DEVICE) {
		rdm_sub_devices_set_dmx_start_address(sub_device, start_address);
		return;
	}

	rdm_device_info.dmx_start_address[0] = (uint8_t) (start_address >> 8);
	rdm_device_info.dmx_start_address[1] = (uint8_t) start_address;
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @return
 */
const uint8_t rdm_device_info_get_personality_count(/*@unused@*/const uint16_t sub_device) {
	return (sizeof(rdm_personalities) / sizeof(struct _rdm_personality));
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @return
 */
const uint8_t rdm_device_info_get_personality_current(const uint16_t sub_device) {
	if (sub_device != RDM_ROOT_DEVICE) {
		return rdm_sub_devices_get_personality_current(sub_device);
	}

	return rdm_device_info.current_personality;
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @param personality
 */
void rdm_device_info_set_personality_current(const uint16_t sub_device, const uint8_t personality) {
	if (sub_device != RDM_ROOT_DEVICE) {
		rdm_sub_devices_set_personality_current(sub_device, personality);
		return;
	}

	rdm_device_info.current_personality = personality;
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @param personality
 * @return
 */
const char *rdm_device_info_get_personality_description(const uint16_t sub_device, const uint8_t personality) {
	if ((personality == 0) || (personality > rdm_device_info_get_personality_count(sub_device))) {
		return NULL;
	}

	if (sub_device != RDM_ROOT_DEVICE) {
		return rdm_sub_devices_get_personality_description(sub_device, personality);
	}

	return (rdm_personalities[personality - 1].description);
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @param personality
 * @return
 */
const uint8_t rdm_device_info_get_personality_description_length(const uint16_t sub_device, const uint8_t personality) {
	if ((personality == 0) || (personality > rdm_device_info_get_personality_count(sub_device))) {
		return 0;
	}

	if (sub_device != RDM_ROOT_DEVICE) {
		return rdm_sub_devices_get_personality_description_length(sub_device, personality);
	}

	return (rdm_personalities[personality - 1].description_len);
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @param personality
 * @return
 */
const uint16_t rdm_device_info_get_personality_slots(const uint16_t sub_device, const uint8_t personality) {
	if ((personality == 0) || (personality > rdm_device_info_get_personality_count(sub_device))) {
		return 0;
	}

	if (sub_device != RDM_ROOT_DEVICE) {
		return rdm_sub_devices_get_personality_slots(sub_device, personality);
	}

	return (rdm_personalities[personality - 1].slots);
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @return
 */
struct _rdm_device_info *rdm_device_info_get(const uint16_t sub_device) {
	if (sub_device != RDM_ROOT_DEVICE) {
		const struct _rdm_sub_devices_info *sub_device_info = rdm_sub_devices_info_get(sub_device);

		if (sub_device_info != NULL) {
			rdm_sub_device_info.dmx_footprint[0] = (uint8_t) (sub_device_info->dmx_footprint >> 8);
			rdm_sub_device_info.dmx_footprint[1] = (uint8_t) sub_device_info->dmx_footprint;
			rdm_sub_device_info.current_personality = sub_device_info->current_personality;
			rdm_sub_device_info.personality_count = sub_device_info->personality_count;
			rdm_sub_device_info.dmx_start_address[0] = (uint8_t) (sub_device_info->dmx_start_address >> 8);
			rdm_sub_device_info.dmx_start_address[1] = (uint8_t) sub_device_info->dmx_start_address;
			rdm_sub_device_info.sensor_count = sub_device_info->sensor_count;
		}

		return &rdm_sub_device_info;
	}

	return &rdm_device_info;
}
#endif

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @param info
 */
void rdm_device_info_get_label(const uint16_t sub_device, struct _rdm_device_info_data *info) {
#ifdef RDM_RESPONDER
	if (sub_device != RDM_ROOT_DEVICE) {
		rdm_sub_devices_get_label(sub_device, info);
		return;
	}
#endif
	info->data = (uint8_t *)root_device_label;
	info->length = root_device_label_length;
}

/**
 * @ingroup rdm
 *
 * @param info
 */
void rdm_device_info_get_manufacturer_name(struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)device_manufacturer_name;
	info->length = device_manufacturer_name_length;
}

/**
 * @ingroup rdm
 *
 * @param info
 */
void rdm_device_info_get_manufacturer_id(struct _rdm_device_info_data *info) {
	manufacturer_id[0] = uid_device[1];
	manufacturer_id[1] = uid_device[0];

	info->data = (uint8_t *)manufacturer_id;
	info->length = RDM_DEVICE_MANUFACTURER_ID_LENGTH;
}

/**
 * @ingroup rdm
 *
 * @param info
 */
void rdm_device_info_get_sn(struct _rdm_device_info_data *info) {
	info->data = (uint8_t *)device_sn;
	info->length = DEVICE_SN_LENGTH;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t * rdm_device_info_get_uuid(void) {
	return uid_device;
}

/**
 * @ingroup rdm
 *
 */
#ifdef RDM_RESPONDER
void rdm_device_info_init(const bool init) {
#else
void rdm_device_info_init(void) {
#endif
	uint8_t mac_address[6] ALIGNED;

#ifdef RDM_RESPONDER
	uint16_t device_model;
	uint32_t software_version_id;
	uint16_t sub_device_count;
	const int32_t board_model_id = hardware_board_get_model_id();

	if (board_model_id < 0) {
		device_model = (uint16_t) 0;
	} else {
		device_model = (uint16_t) board_model_id;
	}
#endif

	if (hardware_get_mac_address(mac_address) == 0) {
		uid_device[2] = mac_address[2];
		uid_device[3] = mac_address[3];
		uid_device[4] = mac_address[4];
		uid_device[5] = mac_address[5];
	}

	uid_device[0] = DEVICE_MANUFACTURER_ID[0];
	uid_device[1] = DEVICE_MANUFACTURER_ID[1];

	device_sn[0] = uid_device[5];
	device_sn[1] = uid_device[4];
	device_sn[2] = uid_device[3];
	device_sn[3] = uid_device[2];

		(void *)memcpy(root_device_label, DEVICE_LABEL, DEVICE_LABEL_LENGTH);
		root_device_label_length = DEVICE_LABEL_LENGTH;

	(void *)memcpy(device_manufacturer_name, DEVICE_MANUFACTURER_NAME, DEVICE_MANUFACTURER_NAME_LENGTH);
	device_manufacturer_name_length = DEVICE_MANUFACTURER_NAME_LENGTH;

	read_config_file(RDM_DEVICE_FILE_NAME, &process_line_read_string);

#ifdef RDM_RESPONDER
	rdm_identify_on();
	if (init) {
		rdm_sensors_init();
		rdm_sub_devices_info_init();
	}

	software_version_id = rdm_device_info_get_software_version_id();
	sub_device_count = rdm_sub_devices_get();

	rdm_device_info.protocol_major = (uint8_t)(E120_PROTOCOL_VERSION >> 8);
	rdm_device_info.protocol_minor = (uint8_t) E120_PROTOCOL_VERSION;
	rdm_device_info.device_model[0] = (uint8_t) (device_model >> 8);
	rdm_device_info.device_model[1] = (uint8_t) device_model;
	rdm_device_info.product_category[0] = (uint8_t) (product_category >> 8);
	rdm_device_info.product_category[1] = (uint8_t) product_category;
	rdm_device_info.software_version[0] = (uint8_t) (software_version_id >> 24);
	rdm_device_info.software_version[1] = (uint8_t) (software_version_id >> 16);
	rdm_device_info.software_version[2] = (uint8_t) (software_version_id >> 8);
	rdm_device_info.software_version[3] = (uint8_t) software_version_id;
	rdm_device_info.dmx_footprint[0] = (uint8_t) (DMX_FOOTPRINT >> 8);
	rdm_device_info.dmx_footprint[1] = (uint8_t) DMX_FOOTPRINT;
	rdm_device_info.dmx_start_address[0] = (uint8_t) (DEFAULT_DMX_START_ADDRESS >> 8);
	rdm_device_info.dmx_start_address[1] = (uint8_t) DEFAULT_DMX_START_ADDRESS;
	rdm_device_info.current_personality = DEFAULT_CURRENT_PERSONALITY;
	rdm_device_info.personality_count = rdm_device_info_get_personality_count(0);
	rdm_device_info.sub_device_count[0] = (uint8_t) (sub_device_count >> 8);
	rdm_device_info.sub_device_count[1] = (uint8_t) sub_device_count;
	rdm_device_info.sensor_count = rdm_sensors_get_count();

	(void *) memcpy(&rdm_sub_device_info, &rdm_device_info, sizeof(struct _rdm_device_info));

	factory_defaults_checksum = calculate_checksum();
	is_factory_defaults = true;
	rdm_identify_off();
#endif
}
