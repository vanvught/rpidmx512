/**
 * @file bcm2835_vc.c
 *
 * https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface\n
 * ARM to VC
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

#include "bcm2835_mailbox.h"
#include "bcm2835_vc.h"

struct vc_msg_tag {
	uint32_t tag_id;		///< the message id
	uint32_t buffer_size;	///< size of the buffer (which in this case is always 8 bytes)
	uint32_t data_size;		///< amount of data being sent or received
	uint32_t dev_id;		///< the ID of the clock/voltage to get or set
	uint32_t val;			///< the value (e.g. rate (in Hz)) to set
};

// TODO rename
struct vc_msg_get {
	uint32_t msg_size;			///< simply, sizeof(struct vc_msg)
	uint32_t request_code;		///< holds various information like the success and number of bytes returned (refer to mailboxes wiki)
	struct vc_msg_tag tag;		///< the tag structure above to make
	uint32_t end_tag;			///< an end identifier, should be set to NULL
};

/**
 * @param tag_id
 * @param dev_id
 * @return
 */
inline static int32_t bcm2835_vc_get(const uint32_t tag_id, const uint32_t dev_id) {

	uint32_t mb_addr = 0x40007000;		// 0x7000 in L2 cache coherent mode
	volatile struct vc_msg_get *vc_msg = (struct vc_msg_get *)mb_addr;

	vc_msg->msg_size = sizeof(struct vc_msg_get);
	vc_msg->request_code = 0;
	vc_msg->tag.tag_id = tag_id;
	vc_msg->tag.buffer_size = 8;
	vc_msg->tag.data_size = 4;	/* we're just sending the clock ID which is one word long */
	vc_msg->tag.dev_id = dev_id;
	vc_msg->tag.val = 0;
	vc_msg->end_tag = 0;

	bcm2835_mailbox_write(BCM2835_MAILBOX_PROP_CHANNEL, mb_addr);
	bcm2835_mailbox_read(BCM2835_MAILBOX_PROP_CHANNEL);

	if (vc_msg->request_code != BCM2835_MAILBOX_SUCCESS) {
		return -1;
	}

	if (vc_msg->tag.dev_id != dev_id) {
		return -1;
	}

	return vc_msg->tag.val;

}

/**
 * @ingroup VideoCore
 *
 * @param tag_id
 * @param dev_id
 * @param val
 * @return
 */
inline static int32_t bcm2835_vc_set(const uint32_t tag_id, const uint32_t dev_id, const uint32_t val) {
	uint32_t mb_addr = 0x40007000;		// 0x7000 in L2 cache coherent mode
	volatile struct  vc_msg_get *vc_msg = (struct vc_msg_get *)mb_addr;

	vc_msg->msg_size = sizeof(struct vc_msg_get);
	vc_msg->request_code = 0;
	vc_msg->tag.tag_id = tag_id;
	vc_msg->tag.buffer_size = 8;
	vc_msg->tag.data_size = 8; /* we're sending the clock ID and the new rates which is a total of 2 words */
	vc_msg->tag.dev_id = dev_id;
	vc_msg->tag.val = val;
	vc_msg->end_tag = 0;

	bcm2835_mailbox_write(BCM2835_MAILBOX_PROP_CHANNEL, mb_addr);
	bcm2835_mailbox_read(BCM2835_MAILBOX_PROP_CHANNEL);

	if (vc_msg->request_code != BCM2835_MAILBOX_SUCCESS) {
		return -1;
	}

	if (vc_msg->tag.dev_id != dev_id) {
		return -1;
	}

	return vc_msg->tag.val;
}

/**
 * @ingroup VideoCore
 *
 * @param clock_id
 *
 * @return rate (in Hz)
 */
int32_t bcm2835_vc_get_clock_rate(const uint32_t clock_id) {
	return bcm2835_vc_get(BCM2835_VC_TAG_GET_CLOCK_RATE, clock_id);
}

/**
 * @ingroup VideoCore
 *
 * @param clock_id
 *
 * @param clock_rate rate (in Hz)
 *
 * @return rate (in Hz). A rate of 0 is returned if the clock does not exist.
 */
int32_t bcm2835_vc_set_clock_rate(const uint32_t clock_id, const uint32_t clock_rate) {
	return bcm2835_vc_set(BCM2835_VC_TAG_SET_CLOCK_RATE, clock_id, clock_rate);
}

/**
 * @ingroup VideoCore
 *
 * @param dev_id
 *
 * @return
 *   Bit 0: 0=off, 1=on
 *   Bit 1: 0=device exists, 1=device does not exist
 *   Bits 2-31: reserved for future use
 */
int32_t bcm2835_vc_get_power_state(const uint32_t dev_id) {
	return (bcm2835_vc_get(BCM2835_VC_TAG_GET_POWER_STATE, dev_id) & 0x3);
}

/**
 * @ingroup VideoCore
 *
 * @param dev_id
 *
 * @param state
 *  Bit 0: 0=off, 1=on
 *  Bit 1: 0=do not wait, 1=wait
 *  Bits 2-31: reserved for future use (set to 0)
 *
 * @return
 *  Bit 0: 0=off, 1=on
 *  Bit 1: 0=device exists, 1=device does not exist
 *  Bits 2-31: reserved for future use
 *
 */
int32_t bcm2835_vc_set_power_state(const uint32_t dev_id, const uint32_t state) {
	return bcm2835_vc_set(BCM2835_VC_TAG_SET_POWER_STATE, dev_id, state);
}
