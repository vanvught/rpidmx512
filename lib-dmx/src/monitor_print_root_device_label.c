/**
 * @file monitor.c
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include "console.h"

#include "rdm.h"
#include "rdm_device_info.h"

static uint8_t rdm_device_info_label_length_previous = (uint8_t)0;

void monitor_print_root_device_label(void) {
	struct _rdm_device_info_data rdm_device_info_label;
	rdm_device_info_get_label(RDM_ROOT_DEVICE, &rdm_device_info_label);
	console_set_cursor(37, 2);
	console_write((const char *)rdm_device_info_label.data, (int) rdm_device_info_label.length);
	if (rdm_device_info_label_length_previous != rdm_device_info_label.length) {
		console_write((const char *)"                                ", RDM_DEVICE_LABEL_MAX_LENGTH - rdm_device_info_label.length);
		rdm_device_info_label_length_previous = rdm_device_info_label.length;
	}
}
