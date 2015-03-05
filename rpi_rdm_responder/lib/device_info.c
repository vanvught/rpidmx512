/**
 * @file device_info.c
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

#include "bcm2835_vc.h"

#include "rdm.h"

// RESERVED FOR PROTOTYPING/EXPERIMENTAL USE ONLY
static uint8_t uid_device[UID_SIZE] = { 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x01 };

static uint16_t dmx_footprint = 32;
static uint16_t dmx_start_address = 1;
static uint16_t current_personality = 1;

static struct _rdm_personality rdm_personalities[] =
			{{ 32, "RDM Responder / DMX Analyzer" }};

void device_info_init(void)
{
	uint8_t mac_address[6];
	if (bcm2835_vc_get_board_mac_address(mac_address) == 0){
		uid_device[2] = mac_address[2];
		uid_device[3] = mac_address[3];
		uid_device[4] = mac_address[4];
		uid_device[5] = mac_address[5];
	}
}

uint8_t * device_info_uuid_get(void)
{
	return uid_device;
}

uint16_t device_info_dmx_footprint_get(void)
{
	return dmx_footprint;
}

uint16_t device_info_dmx_start_address_get(void)
{
	return dmx_start_address;
}

void device_info_dmx_start_address_set(uint16_t start_address)
{
	if (start_address == 0 ||  start_address > 512)
		return;

	dmx_start_address = start_address;
}

uint8_t device_info_personality_count_get(void)
{
	return (sizeof(rdm_personalities) / sizeof(struct _rdm_personality));
}

uint8_t device_info_current_personality_get(void)
{
	return current_personality;
}

void device_info_current_personality_set(uint8_t personality)
{
	if ((personality == 0) || (personality > device_info_personality_count_get()))
			return;

	current_personality = personality;
}

const char * device_info_personality_description_get(uint8_t personality)
{
	if ((personality == 0) || (personality > device_info_personality_count_get()))
			return NULL;

	personality--;

	return (rdm_personalities[personality].description);
}

uint16_t device_info_personality_slots_get(uint8_t personality)
{
	if ((personality == 0) || (personality > device_info_personality_count_get()))
			return 0;

	personality--;

	return (rdm_personalities[personality].slots);
}
