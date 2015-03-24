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

#include "hardware.h"
#include "bcm2835_vc.h"
#include "util.h"
#include "rdm.h"
#include "rdm_e120.h"
#include "rdm_device_info.h"
#include "rdm_device_const.h"

static const uint8_t  DEVICE_LABEL_LENGTH = (sizeof(DEVICE_LABEL) / sizeof(uint8_t)) - 1;
static const uint8_t  DEVICE_MANUFACTURER_NAME_LENGTH = (sizeof(DEVICE_MANUFACTURER_NAME) / sizeof(uint8_t)) - 1;
static const uint8_t  DEVICE_SOFTWARE_VERSION_LENGTH = (sizeof(DEVICE_SOFTWARE_VERSION) / sizeof(uint8_t)) - 1;

// 0x7F, 0xF0 : RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static uint8_t uid_device[RDM_UID_SIZE] = { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00 };
static uint8_t device_label[DEVICE_LABEL_MAX_LENGTH];
static uint8_t device_label_length = 0;

static uint8_t device_sn[DEVICE_SN_LENGTH];

static uint8_t is_factory_defaults = TRUE;
static uint16_t factory_defaults_checksum = 0;

static struct _rdm_device_info rdm_device_info;

inline static uint16_t calculate_checksum(void)
{
	uint16_t checksum = (rdm_device_info.dmx_start_address[0] >> 8) + rdm_device_info.dmx_start_address[1];
	checksum += rdm_device_info.current_personality;

	uint8_t i = 0;
	for( i= 0; i < device_label_length; i++)
		checksum += device_label[i];

	return checksum;
}

uint8_t rdm_device_info_get_is_factory_defaults()
{
	if (is_factory_defaults == TRUE)
	{
		is_factory_defaults = (factory_defaults_checksum == calculate_checksum());
	}
	return is_factory_defaults;
}

/**
 * @ingroup device_info
 *
 * @return
 */
const uint8_t * rdm_device_info_get_uuid(void)
{
	return uid_device;
}

const uint8_t * rdm_device_info_get_sn(void)
{
	return device_sn;
}

const uint8_t rdm_device_info_get_sn_length(void)
{
	return 4;
}

const uint8_t * rdm_device_info_get_label(void)
{
	return device_label;
}

void rdm_device_info_set_label(const uint8_t *label, uint8_t label_length)
{
	if (label_length > (sizeof(device_label) / sizeof(uint8_t)))
	{
		label_length = sizeof(device_label) / sizeof(uint8_t);
	}

	memcpy(device_label, label, label_length);
	device_label_length = label_length;
}

const uint8_t rdm_device_info_get_label_length(void)
{
	return device_label_length;
}


const uint8_t * rdm_device_info_get_manufacturer_name(void)
{
	return DEVICE_MANUFACTURER_NAME;
}

const uint8_t rdm_device_info_get_manufacturer_name_length(void)
{
	return DEVICE_MANUFACTURER_NAME_LENGTH;
}

const uint8_t * rdm_device_info_get_manufacturer_id(void)
{
	return DEVICE_MANUFACTURER_ID;
}

const uint8_t rdm_device_info_get_manufacturer_id_length(void)
{
	return DEVICE_MANUFACTURER_ID_LENGTH;
}


const uint8_t * rdm_device_info_get_supported_language(void)
{
	return DEVICE_SUPPORTED_LANGUAGE;
}

const uint8_t rdm_device_info_get_supported_language_length(void)
{
	return DEVICE_SUPPORTED_LANGUAGE_LENGTH;
}

const uint32_t rdm_device_info_get_software_version_id(void)
{
	return DEVICE_SOFTWARE_VERSION_ID;
}

const uint8_t * rdm_device_info_get_software_version(void)
{
	return DEVICE_SOFTWARE_VERSION;
}

const uint8_t rdm_device_info_get_software_version_length(void)
{
	return DEVICE_SOFTWARE_VERSION_LENGTH;
}

/**
 * @ingroup device_info
 *
 * @return
 */
uint16_t rdm_device_info_get_dmx_footprint(void)
{
	uint16_t dmx_footprint = (rdm_device_info.dmx_footprint[0] << 8) + rdm_device_info.dmx_footprint[1];
	return dmx_footprint;
}

/**
 * @ingroup device_info
 *
 * @return
 */
uint16_t rdm_device_info_get_dmx_start_address(void)
{
	uint16_t dmx_start_address = (rdm_device_info.dmx_start_address[0] << 8) + rdm_device_info.dmx_start_address[1];
	return dmx_start_address;
}

/**
 * @ingroup device_info
 *
 * @param start_address
 */
void rdm_device_info_set_dmx_start_address(const uint16_t start_address)
{
	if (start_address == 0 ||  start_address > 512)
		return;

	rdm_device_info.dmx_start_address[0] = (uint8_t)(start_address >> 8);
	rdm_device_info.dmx_start_address[1] = (uint8_t)start_address;
}


/**
 * @ingroup device_info
 *
 * @return
 */
uint8_t rdm_device_info_get_personality_count(void)
{
	return (sizeof(rdm_personalities) / sizeof(struct _rdm_personality));
}

/**
 * @ingroup device_info
 *
 * @return
 */
uint8_t rdm_device_info_get_personality_current(void)
{
	return rdm_device_info.current_personality;
}

/**
 * @ingroup device_info
 *
 * @param personality
 */
void rdm_device_info_set_personality_current(const uint8_t personality)
{
	if ((personality == 0) || (personality > rdm_device_info_get_personality_count()))
			return;

	rdm_device_info.current_personality = personality;
}

/**
 * @ingroup device_info
 *
 * @param personality
 * @return
 */
const char * rdm_device_info_get_personality_description(uint8_t personality)
{
	if ((personality == 0) || (personality > rdm_device_info_get_personality_count()))
			return NULL;

	personality--;

	return (rdm_personalities[personality].description);
}

/**
 * @ingroup device_info
 *
 * @param personality
 * @return
 */
uint16_t rdm_device_info_get_personality_slots(uint8_t personality)
{
	if ((personality == 0) || (personality > rdm_device_info_get_personality_count()))
			return 0;

	personality--;

	return (rdm_personalities[personality].slots);
}


struct _rdm_device_info *rdm_device_info_get(void)
{
	return &rdm_device_info;
}

/**
 * @ingroup device_info
 *
 */
void rdm_device_info_init(void)
{
	uint8_t mac_address[6];
	if (bcm2835_vc_get_board_mac_address(mac_address) == 0){
		uid_device[2] = mac_address[2];
		uid_device[3] = mac_address[3];
		uid_device[4] = mac_address[4];
		uid_device[5] = mac_address[5];
	}

	uid_device[0] = DEVICE_MANUFACTURER_ID[1];
	uid_device[1] = DEVICE_MANUFACTURER_ID[0];

	device_sn[0] = uid_device[5];
	device_sn[1] = uid_device[4];
	device_sn[2] = uid_device[3];
	device_sn[3] = uid_device[2];

	memcpy(device_label, DEVICE_LABEL, DEVICE_LABEL_LENGTH);
	device_label_length = DEVICE_LABEL_LENGTH;

	uint16_t device_model = hardware_get_board_model_id();
	uint32_t software_version_id = rdm_device_info_get_software_version_id();

	rdm_device_info.protocol_major       = E120_PROTOCOL_VERSION >> 8;
	rdm_device_info.protocol_minor       = (uint8_t)E120_PROTOCOL_VERSION;
	rdm_device_info.device_model[0]      = (uint8_t)(device_model >> 8);
	rdm_device_info.device_model[1]      = (uint8_t)device_model;
	rdm_device_info.product_category[0]  = (uint8_t)(E120_PRODUCT_DETAIL_OTHER >> 8);
	rdm_device_info.product_category[1]  = (uint8_t)E120_PRODUCT_DETAIL_OTHER;
	rdm_device_info.software_version[0]  = (uint8_t)(software_version_id >> 24);
	rdm_device_info.software_version[1]  = (uint8_t)(software_version_id >> 16);
	rdm_device_info.software_version[2]  = (uint8_t)(software_version_id >> 8);
	rdm_device_info.software_version[3]  = (uint8_t)software_version_id;
	rdm_device_info.dmx_footprint[0]     = (uint8_t)(DMX_FOOTPRINT >> 8);
	rdm_device_info.dmx_footprint[1]     = (uint8_t)DMX_FOOTPRINT;
	rdm_device_info.dmx_start_address[0] = (uint8_t)(DEFAULT_DMX_START_ADDRESS >> 8);
	rdm_device_info.dmx_start_address[1] = (uint8_t)DEFAULT_DMX_START_ADDRESS;
	rdm_device_info.current_personality  = DEFAULT_CURRENT_PERSONALITY;
	rdm_device_info.personality_count    = rdm_device_info_get_personality_count();
	rdm_device_info.sub_device_count[0]  = 0;
	rdm_device_info.sub_device_count[1]  = 0;
	rdm_device_info.sensor_count         = 0;

	factory_defaults_checksum = calculate_checksum();
	is_factory_defaults = TRUE;
}
