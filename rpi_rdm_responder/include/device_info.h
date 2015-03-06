/**
 * @file device_info.h
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

#ifndef DEVICE_INFO_H_
#define DEVICE_INFO_H_

#include <stdint.h>

extern void device_info_init(void);

extern uint8_t * device_info_uuid_get(void);

extern const uint8_t * device_info_label_get(void);
extern const uint8_t device_info_label_length_get(void);
extern void device_info_label_set(const uint8_t *, uint8_t );
extern uint8_t device_info_is_factory_defaults_get(void);
extern const uint8_t * device_info_manufacturer_name_get(void);
extern const uint8_t device_info_manufacturer_name_length_get(void);
extern const uint8_t * device_info_supported_language_get(void);
extern const uint8_t device_info_supported_language_length_get(void);
extern const uint8_t * device_info_software_version_get(void);
extern const uint8_t device_info_software_version_length_get(void);
extern uint16_t device_info_dmx_footprint_get(void);
extern uint16_t device_info_dmx_start_address_get(void);
extern void device_info_dmx_start_address_set(uint16_t);
extern uint8_t device_info_personality_count_get(void);
extern uint8_t device_info_current_personality_get(void);
extern void device_info_current_personality_set(uint8_t);
extern const char * device_info_personality_description_get(uint8_t);
extern uint16_t device_info_personality_slots_get(uint8_t);

#endif /* DEVICE_INFO_H_ */
