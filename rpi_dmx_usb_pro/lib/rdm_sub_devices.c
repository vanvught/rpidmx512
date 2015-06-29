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

static uint16_t rdm_sub_devices = 0;		///< This RDM Responder does not have sub-devices.

/**
 *
 * @param sub_device
 * @return
 */
const uint16_t rdm_sub_devices_get_footprint(/*@unused@*/const uint16_t sub_device) {
	return 0;
}

/**
 *
 * @param sub_device
 * @return
 */
const uint16_t rdm_sub_devices_get_dmx_start_address(/*@unused@*/const uint16_t sub_device) {
	return 0;
}

/**
 *
 * @param sub_device
 * @param dmx_start_address
 */
void rdm_sub_devices_set_dmx_start_address(/*@unused@*/const uint16_t sub_device, /*@unused@*/const uint16_t dmx_start_address) {

}

/**
 *
 * @param sub_device
 * @return
 */
const char *rdm_sub_devices_get_label(/*@unused@*/const uint16_t sub_device) {
	return NULL;
}

/**
 *
 * @param sub_device
 * @param label
 * @param label_length
 */
void rdm_sub_devices_set_label(const /*@unused@*/uint16_t sub_device, /*@unused@*/const uint8_t *label, /*@unused@*/uint8_t label_length) {

}

/**
 *
 * @param sub_device
 * @return
 */
const uint8_t rdm_sub_devices_get_label_length(/*@unused@*/const uint16_t sub_device) {
	return 0;
}


/**
 *
 * @param sub_device
 * @return
 */
const uint8_t rdm_sub_devices_get_personality_current(/*@unused@*/const uint16_t sub_device) {
	return 0;
}

/**
 *
 * @param sub_device
 * @param personality
 */
void rdm_sub_devices_set_personality_current(/*@unused@*/const uint16_t sub_device, /*@unused@*/const uint8_t personality) {

}

/**
 *
 * @param sub_device
 * @param personality
 * @return
 */
const char *rdm_sub_devices_get_personality_description(/*@unused@*/const uint16_t sub_device, /*@unused@*/const uint8_t personality) {
	return NULL;
}

/**
 *
 * @param sub_device
 * @param personality
 * @return
 */
const uint16_t rdm_sub_devices_get_personality_slots(/*@unused@*/const uint16_t sub_device, /*@unused@*/const uint8_t personality) {
	return 0;
}

/**
 *
 * @param sub_device
 * @return
 */
const struct _rdm_sub_devices_info *rdm_sub_devices_info_get(/*@unused@*/const uint16_t sub_device) {
	return NULL;
}

/**
 *
 * @param sub_device
 * @param sub_devices_info
 */
void rdm_sub_devices_info_set(/*@unused@*/const uint16_t sub_device, /*@unused@*/const struct _rdm_sub_devices_info *sub_devices_info) {

}

/**
 *
 * @return
 */
const uint16_t rdm_sub_devices_get(void) {
	return rdm_sub_devices;
}



