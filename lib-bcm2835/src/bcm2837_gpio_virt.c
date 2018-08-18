/**
 * @file bcm2837_gpio_virt.c
 *
 */
/* This code is inspired by:
 *
 * https://github.com/raspberrypi/linux/blob/02ce9572cc77c65f49086bbc4281233bd3fa48b7/drivers/gpio/gpio-bcm-virt.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "arm/synchronize.h"

#include "bcm2835.h"
#include "bcm2835_mailbox.h"

#define RPI_FIRMWARE_FRAMEBUFFER_GET_GPIO_VIRTBUF	0x00040010
#define RPI_FIRMWARE_FRAMEBUFFER_SET_GPIO_VIRTBUF	0x00048020

#define NUM_GPIO 2

static volatile uint32_t gpiovirtbuf;
static uint32_t enables_disables[NUM_GPIO];

struct vc_msg_tag_uint32_t {
	uint32_t tag_id;		///< the message id
	uint32_t buffer_size;	///< size of the buffer
	uint32_t data_size;		///< amount of data being sent or received
	uint32_t value;			///<

};

struct vc_msg_uint32_t {
	uint32_t msg_size;				///< simply, sizeof(struct vc_msg)
	uint32_t request_code;			///< holds various information like the success and number of bytes returned (refer to mailboxes wiki)
	struct vc_msg_tag_uint32_t tag;	///< the tag structure above to make
	uint32_t end_tag;				///< an end identifier, should be set to NULL
};

static uint32_t firmware_property_get(void) {
	struct vc_msg_uint32_t *vc_msg = (struct vc_msg_uint32_t *)MEM_COHERENT_REGION;
	uint32_t address = GPU_MEM_BASE + (uint32_t) vc_msg;

	vc_msg->msg_size = sizeof(struct vc_msg_uint32_t);
	vc_msg->request_code = 0;
	vc_msg->tag.tag_id = RPI_FIRMWARE_FRAMEBUFFER_GET_GPIO_VIRTBUF;
	vc_msg->tag.buffer_size = 4;
	vc_msg->tag.data_size = 0;
	vc_msg->tag.value = 0;
	vc_msg->end_tag = 0;

	if (bcm2835_mailbox_write_read(BCM2835_MAILBOX_PROP_CHANNEL, address) != address) {
		return 0;
	}

	if (vc_msg->request_code != BCM2835_MAILBOX_SUCCESS) {
		return 0;
	}

	return vc_msg->tag.value;
}

static uint32_t firmware_property_set(void) {
	struct vc_msg_uint32_t *vc_msg = (struct vc_msg_uint32_t *)MEM_COHERENT_REGION;
	uint32_t address = GPU_MEM_BASE + (uint32_t)vc_msg;

	vc_msg->msg_size = sizeof(struct vc_msg_uint32_t);
	vc_msg->request_code = 0;
	vc_msg->tag.tag_id = RPI_FIRMWARE_FRAMEBUFFER_SET_GPIO_VIRTBUF;
	vc_msg->tag.buffer_size = 8;
	vc_msg->tag.data_size = 4;
	vc_msg->tag.value = address + 4096; // FIXME
	vc_msg->end_tag = 0;

	if (bcm2835_mailbox_write_read(BCM2835_MAILBOX_PROP_CHANNEL, address) != address) {
		return 0;
	}

	if (vc_msg->request_code != BCM2835_MAILBOX_SUCCESS) {
		return 0;
	}

	return vc_msg->tag.value;
}

uint32_t bcm2837_gpio_virt_get_address(void) {
	return gpiovirtbuf;
}

void bcm2837_gpio_virt_init(void) {
	int i;

	for (i = 0 ; i < NUM_GPIO ; i++) {
		enables_disables[i] = 0;
	}

	if (firmware_property_set() == 0) {
		if ((gpiovirtbuf = firmware_property_get()) != 0) {
			gpiovirtbuf &= 0x3FFFFFFF;
		}
	}

}

void bcm2837_gpio_virt_led_set(int val) {
	uint16_t enables, disables;
	int16_t diff;
	bool lit;

	if (gpiovirtbuf == 0) {
		return;
	}

	enables = enables_disables[0] >> 16;
	disables = enables_disables[0] >> 0;

	diff = (int16_t) (enables - disables);
	lit = diff > 0;

	if ((val && lit) || (!val && !lit)) {
		return;
	}

	if (val) {
		enables++;
	} else {
		disables++;
	}

	enables_disables[0] = (enables << 16) | (disables << 0);

	dsb();
	*(volatile uint32_t *) gpiovirtbuf = enables_disables[0];
	dmb();
}
