/**
 * @file bcm2835_mailbox.h
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

#ifndef BCM2835_MAILBOX_H_
#define BCM2835_MAILBOX_H_

#include <stdint.h>

#define BCM2835_MAILBOX_SUCCESS	(uint32_t)0x80000000	///< Request successful
#define BCM2835_MAILBOX_ERROR	(uint32_t)0x80000001	///< Error parsing request buffer (partial response)

/**
 * @brief The following lists the currently defined mailbox channels.
 */
typedef enum {
	BCM2835_MAILBOX_POWER_CHANNEL 	= 0,	///< For use by the power management interface
	BCM2835_MAILBOX_FB_CHANNEL 		= 1,	///< https://github.com/raspberrypi/firmware/wiki/Mailbox-framebuffer-interface
	BCM2835_MAILBOX_VCHIQ_CHANNEL 	= 3,    ///< For use by the VCHIQ interface
	BCM2835_MAILBOX_PROP_CHANNEL 	= 8		///< https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
} bcm2835MailboxChannels;

extern void bcm2835_mailbox_flush(void);
extern uint32_t bcm2835_mailbox_read(uint8_t);
extern void bcm2835_mailbox_write(uint8_t, uint32_t);
extern uint32_t bcm2835_mailbox_write_read(uint8_t channel, uint32_t data);

#endif /* BCM2835_MAILBOX_H_ */
