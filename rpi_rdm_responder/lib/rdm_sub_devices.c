/**
 * @file rdm_sub_devices.c
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

#include "rdm.h"
#include "rdm_sub_devices.h"

/*
uint16_t dmx_footprint;
uint8_t current_personality;
uint8_t personality_count;
uint16_t dmx_start_address;
uint8_t sensor_count;
uint8_t device_label[DEVICE_LABEL_MAX_LENGTH];
uint8_t device_label_length;
*/

static const struct _rdm_personality rdm_sub_device_1[] = {
		{ 7, "Digital output 7-lines" }
		};

static const struct _rdm_personality rdm_sub_device_2[] = {
		{ 2, "2 Relays" }
		};

static const struct _rdm_personality rdm_sub_device_34[] = {
		{ 8, "Digital output 8-lines" }
		};

static struct _rdm_sub_devices_info rdm_sub_devices_info[] = {
		{ 7, 1, 1,  1, 0, "bw_spi_dio", 10, &rdm_sub_device_1[0] },
		{ 2, 1, 1,  1, 0, "bw_spi_relay", 12, &rdm_sub_device_2[0] },
		{ 8, 1, 1,  9, 0, "mcp23s08", 8, &rdm_sub_device_34[0] },
		{ 8, 1, 1, 17, 0, "mcp23s08", 8, &rdm_sub_device_34[0] },
};

static uint16_t rdm_sub_devices = 4;

/**
 *
 * @param sub_device
 * @return
 */
const uint16_t rdm_sub_devices_get_footprint(const uint16_t sub_device)
{
	return rdm_sub_devices_info[sub_device - 1].dmx_footprint;
}

/**
 *
 * @param sub_device
 * @return
 */
const uint16_t rdm_sub_devices_get_dmx_start_address(const uint16_t sub_device)
{
	return rdm_sub_devices_info[sub_device - 1].dmx_start_address;
}

/**
 *
 * @param sub_device
 * @param dmx_start_address
 */
void rdm_sub_devices_set_dmx_start_address(const uint16_t sub_device, const uint16_t dmx_start_address)
{
	rdm_sub_devices_info[sub_device - 1].dmx_start_address = dmx_start_address;
}

/**
 *
 * @param sub_device
 * @return
 */
const uint8_t *rdm_sub_devices_get_label(const uint16_t sub_device)
{
	return rdm_sub_devices_info[sub_device - 1].device_label;
}

/**
 *
 * @param sub_device
 * @param label
 * @param label_length
 */
void rdm_sub_devices_set_label(const uint16_t sub_device, const uint8_t *label, uint8_t label_length)
{
	memcpy(rdm_sub_devices_info[sub_device - 1].device_label, label, label_length);
	rdm_sub_devices_info[sub_device - 1].device_label_length = label_length;
}

/**
 *
 * @param sub_device
 * @return
 */
const uint8_t rdm_sub_devices_get_label_length(const uint16_t sub_device)
{
	return rdm_sub_devices_info[sub_device - 1].device_label_length;
}


const uint8_t rdm_sub_devices_get_personality_current(const uint16_t sub_device)
{
	return rdm_sub_devices_info[sub_device - 1].current_personality;
}

/**
 *
 * @param sub_device
 * @param personality
 */
void rdm_sub_devices_set_personality_current(const uint16_t sub_device, const uint8_t personality)
{
	rdm_sub_devices_info[sub_device - 1].current_personality = personality;
}

const char *rdm_sub_devices_get_personality_description(const uint16_t sub_device, const uint8_t personality)
{
	return rdm_sub_devices_info[sub_device - 1].rdm_personalities[personality - 1].description;
}

const uint16_t rdm_sub_devices_get_personality_slots(const uint16_t sub_device, const uint8_t personality)
{
	return rdm_sub_devices_info[sub_device - 1].rdm_personalities[personality - 1].slots;
}

/**
 *
 * @param sub_device
 * @return
 */
const struct _rdm_sub_devices_info *rdm_sub_devices_info_get(const uint16_t sub_device)
{
	return &rdm_sub_devices_info[sub_device - 1];
}

void rdm_sub_devices_info_set(const uint16_t sub_device, const struct _rdm_sub_devices_info *sub_devices_info)
{
	memcpy(&rdm_sub_devices_info[sub_device - 1], sub_devices_info, sizeof(struct _rdm_sub_devices_info));
}

/**
 *
 * @return
 */
const uint16_t rdm_sub_devices_get(void)
{
	return rdm_sub_devices;
}



