/**
 * @file rdm_device_info.c
 *
 */
/* Copyright (C) 2015 by Arjan van Vught <pm @ http://www.raspberrypi.org/forum/>
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
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "ff.h"
#include "hardware.h"
#include "util.h"
#include "dmx.h"
#include "rdm.h"
#include "rdm_e120.h"
#include "rdm_sub_devices.h"
#include "rdm_device_info.h"
#include "rdm_device_const.h"
#include "rdm_sensor.h"
#include "sscan.h"

static const uint8_t DEVICE_LABEL_LENGTH = sizeof(DEVICE_LABEL) / sizeof(DEVICE_LABEL[0]) - 1;
static const uint8_t DEVICE_MANUFACTURER_NAME_LENGTH = sizeof(DEVICE_MANUFACTURER_NAME) / sizeof(DEVICE_MANUFACTURER_NAME[0]) - 1;
#ifdef RDM_RESPONDER
static const uint8_t DEVICE_SOFTWARE_VERSION_LENGTH = sizeof(DEVICE_SOFTWARE_VERSION) / sizeof(DEVICE_SOFTWARE_VERSION[0]) - 1;
#endif

static const TCHAR RDM_DEVICE_FILE_NAME[] = "rdm_device.txt";				///< Parameters file name
static const char RDM_DEVICE_MANUFACTURER_NAME[] = "manufacturer_name";		///<
static const char RDM_DEVICE_MANUFACTURER_ID[] = "manufacturer_id";			///<

// 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static uint8_t uid_device[RDM_UID_SIZE] = { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00 };
static char root_device_label[DEVICE_LABEL_MAX_LENGTH];
static uint8_t root_device_label_length = 0;

static char device_manufacturer_name[DEVICE_MANUFACTURER_LABEL_MAX_LENGTH];
static uint8_t device_manufacturer_name_length = 0;
static uint8_t device_sn[DEVICE_SN_LENGTH];

static uint8_t manufacturer_id[DEVICE_MANUFACTURER_ID_LENGTH];

#ifdef RDM_RESPONDER
static bool is_factory_defaults = true;
static uint16_t factory_defaults_checksum = 0;

static struct _rdm_device_info rdm_device_info __attribute__((aligned(4)));
static struct _rdm_device_info rdm_sub_device_info __attribute__((aligned(4)));
#endif
/**
 * @ingroup rdm
 *
 * Process the input , parse name=value
 *
 * @param line
 */
static void process_line_read_string(const char *line) {
	char value[8] __attribute__((aligned(4)));
	uint8_t len;

	device_manufacturer_name_length = DEVICE_MANUFACTURER_LABEL_MAX_LENGTH;
	(void) sscan_char_p(line, RDM_DEVICE_MANUFACTURER_NAME, device_manufacturer_name, &device_manufacturer_name_length);

	len = 4;
	if (sscan_char_p(line, RDM_DEVICE_MANUFACTURER_ID, value, &len) == 2) {
		if (len == 4) {
			if (isxdigit((int )value[0]) && isxdigit((int)value[1]) && isxdigit((int)value[2]) && isxdigit((int)value[3])) {
				uint8_t nibble_high;
				uint8_t nibble_low;

				nibble_high = (value[0] > '9' ? (value[0] | 0x20) - 'a' + 10 : value[0] - '0') << 4;
				nibble_low = value[1] > '9' ? (value[1] | 0x20) - 'a' + 10 : value[1] - '0';

				uid_device[0] = nibble_high | nibble_low;

				nibble_high = (value[2] > '9' ? (value[2] | 0x20) - 'a' + 10 : value[2] - '0') << 4;
				nibble_low = value[3] > '9' ? (value[3] | 0x20) - 'a' + 10 : value[3] - '0';

				uid_device[1] = nibble_high | nibble_low;
			}
		}
	}

	return;
}

/**
 * @ingroup rdm
 *
 * Read the configuration file from the sdcard and process each line.
 *
 */
static void read_config_file(void) {
	int rc = -1;

	FATFS fat_fs;
	FIL file_object;

	f_mount(0, &fat_fs);		// Register volume work area (never fails)

	rc = f_open(&file_object, RDM_DEVICE_FILE_NAME, FA_READ);

	if (rc == FR_OK) {
		TCHAR buffer[128];
		for (;;) {
			if (f_gets(buffer, sizeof buffer, &file_object) == NULL)
				break; // Error or end of file
			process_line_read_string((const char *) buffer);
		}
		f_close(&file_object);
	} else {
	}

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

	uint16_t checksum = (rdm_device_info.dmx_start_address[0] >> 8) + rdm_device_info.dmx_start_address[1];
	checksum += rdm_device_info.current_personality;

	for (i = 0; i < root_device_label_length; i++) {
		checksum += (uint16_t) root_device_label[i];
	}

	return checksum;
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
	if (label_length > DEVICE_LABEL_MAX_LENGTH) {
		label_length = DEVICE_LABEL_MAX_LENGTH;
	}

	if (sub_device != 0) {
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
	return DEVICE_SUPPORTED_LANGUAGE_LENGTH;
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

	if (sub_device != 0) {
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

	if (sub_device != 0) {
		return rdm_sub_devices_get_dmx_start_address(sub_device);
	}

	dmx_start_address = (rdm_device_info.dmx_start_address[0] << 8)
			+ rdm_device_info.dmx_start_address[1];

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

	if (sub_device != 0) {
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
	if (sub_device != 0) {
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
	if (sub_device != 0) {
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

	if (sub_device != 0) {
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
const uint16_t rdm_device_info_get_personality_slots(const uint16_t sub_device, const uint8_t personality) {
	if ((personality == 0) || (personality > rdm_device_info_get_personality_count(sub_device))) {
		return 0;
	}

	if (sub_device != 0) {
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
	if (sub_device != 0) {
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
 * @return
 */
const char * rdm_device_info_get_label(const uint16_t sub_device) {
#ifdef RDM_RESPONDER
	if (sub_device != 0) {
		return rdm_sub_devices_get_label(sub_device);
	}
#endif
	return root_device_label;
}

/**
 * @ingroup rdm
 *
 * @param sub_device
 * @return
 */
const uint8_t rdm_device_info_get_label_length(const uint16_t sub_device) {
#ifdef RDM_RESPONDER
	if (sub_device != 0) {
		return rdm_sub_devices_get_label_length(sub_device);
	}
#endif
	return root_device_label_length;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const char * rdm_device_info_get_manufacturer_name(void) {
	return device_manufacturer_name;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t rdm_device_info_get_manufacturer_name_length(void) {
	return device_manufacturer_name_length;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t * rdm_device_info_get_manufacturer_id(void) {
	manufacturer_id[0] = uid_device[1];
	manufacturer_id[1] = uid_device[0];

	return manufacturer_id;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t rdm_device_info_get_manufacturer_id_length(void) {
	return DEVICE_MANUFACTURER_ID_LENGTH;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t * rdm_device_info_get_sn(void) {
	return device_sn;
}

/**
 * @ingroup rdm
 *
 * @return
 */
const uint8_t rdm_device_info_get_sn_length(void) {
	return 4;
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
void rdm_device_info_init(void) {
#ifdef RDM_RESPONDER
	uint16_t device_model;
	const int32_t board_model_id = hardware_get_board_model_id();
	const uint32_t software_version_id = rdm_device_info_get_software_version_id();
	const uint16_t sub_device_count = rdm_sub_devices_get();
#endif
	uint8_t mac_address[6];
#ifdef RDM_RESPONDER
	if (board_model_id < 0) {
		device_model = (uint16_t)0;
	} else {
		device_model = (uint16_t)board_model_id;
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

	memcpy(root_device_label, DEVICE_LABEL, DEVICE_LABEL_LENGTH);
	root_device_label_length = DEVICE_LABEL_LENGTH;

	memcpy(device_manufacturer_name, DEVICE_MANUFACTURER_NAME, DEVICE_MANUFACTURER_NAME_LENGTH);
	device_manufacturer_name_length = DEVICE_MANUFACTURER_NAME_LENGTH;

	read_config_file();
#ifdef RDM_RESPONDER

	rdm_device_info.protocol_major = (uint8_t)(E120_PROTOCOL_VERSION >> 8);
	rdm_device_info.protocol_minor = (uint8_t) E120_PROTOCOL_VERSION;
	rdm_device_info.device_model[0] = (uint8_t) (device_model >> 8);
	rdm_device_info.device_model[1] = (uint8_t) device_model;
	rdm_device_info.product_category[0] = (uint8_t) (E120_PRODUCT_DETAIL_OTHER >> 8);
	rdm_device_info.product_category[1] = (uint8_t) E120_PRODUCT_DETAIL_OTHER;
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

	rdm_sensors_init();

	memcpy(&rdm_sub_device_info, &rdm_device_info, sizeof(struct _rdm_device_info));

	factory_defaults_checksum = calculate_checksum();
	is_factory_defaults = true;
#endif
}
