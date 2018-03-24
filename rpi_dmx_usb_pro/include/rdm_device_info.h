/**
 * @file rdm_device_info.h
 *
 */
/* Copyright (C) 2015-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef RDM_DEVICE_INFO_H_
#define RDM_DEVICE_INFO_H_

#include <stdint.h>
#include <stdbool.h>

#include "util.h"

#define DEVICE_SN_LENGTH						4	///<

struct _rdm_device_info_data {
	/*@shared@*//*@null@*/uint8_t *data;
	uint8_t length;
};

extern void rdm_device_info_init(void);

extern/*@shared@*/const uint8_t * rdm_device_info_get_uuid(void);
extern void rdm_device_info_get_label(const uint16_t, /*@out@*/struct _rdm_device_info_data *);
extern void rdm_device_info_get_manufacturer_name(/*@out@*/struct _rdm_device_info_data *);
extern void rdm_device_info_get_manufacturer_id(/*@out@*/struct _rdm_device_info_data *);
extern void rdm_device_info_get_sn(struct _rdm_device_info_data *);
extern const uint8_t rdm_device_info_get_ext_mon_level(void);

#endif /* RDM_DEVICE_INFO_H_ */
