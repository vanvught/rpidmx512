/**
 * @file rdm_device_info.h
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

extern void rdm_device_info_init(const bool);
extern/*@shared@*/const uint8_t * rdm_device_info_get_uuid(void);
extern void rdm_device_info_get_label(const uint16_t, /*@out@*/struct _rdm_device_info_data *);
extern void rdm_device_info_get_manufacturer_name(/*@out@*/struct _rdm_device_info_data *);
extern void rdm_device_info_get_manufacturer_id(/*@out@*/struct _rdm_device_info_data *);
extern void rdm_device_info_get_sn(struct _rdm_device_info_data *);
extern const uint8_t rdm_device_info_get_ext_mon_level(void);

extern /*@shared@*/struct _rdm_device_info *rdm_device_info_get(const uint16_t) ASSUME_ALIGNED;
extern const uint16_t rdm_device_info_get_dmx_footprint(const uint16_t);
extern const uint16_t rdm_device_info_get_dmx_start_address(const uint16_t);
extern const bool rdm_device_info_get_is_factory_defaults(void);
extern const uint8_t rdm_device_info_get_personality_count(const uint16_t);
extern const uint8_t rdm_device_info_get_personality_current(const uint16_t);
extern /*@shared@*//*@null@*/const char * rdm_device_info_get_personality_description(const uint16_t, uint8_t);
extern const uint8_t rdm_device_info_get_personality_description_length(const uint16_t, const uint8_t);
extern const uint16_t rdm_device_info_get_personality_slots(const uint16_t, uint8_t);
extern /*@shared@*/const char * rdm_device_info_get_software_version(void);
extern const uint8_t rdm_device_info_get_software_version_length(void);
extern const uint32_t rdm_device_info_get_software_version_id(void);
extern /*@shared@*/const char * rdm_device_info_get_supported_language(void);
extern const uint8_t rdm_device_info_get_supported_language_length(void);
extern void rdm_device_info_set_dmx_start_address(const uint16_t, const uint16_t);
extern void rdm_device_info_set_label(const uint16_t, const uint8_t *, uint8_t);
extern void rdm_device_info_set_personality_current(const uint16_t, const uint8_t);

extern const uint16_t rdm_device_info_get_product_category(void);
extern const uint16_t rdm_device_info_get_product_detail(void);

#endif /* RDM_DEVICE_INFO_H_ */
