/**
 * @file hardware_uuid.c
 */
/* Copyright (C) 2016, 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdio.h>

#include "uuid/uuid.h"

#include "hardware.h"
#include "ff.h"
#include "util.h"

static const char NO_MAC[] ALIGNED = "nomacadr";
static const char EXT_UID[] ALIGNED = ".uid";
static const char DUMMY_UUID[] ALIGNED = "01234567-89ab-cdef-0134-567890abcedf";

const bool hardware_uuid(uuid_t out) {
	char file_name[16];	///< mac:8, .:1 ,uud:3 ,'\0':1
	uint8_t mac[8];
	bool have_uuid = false;

	FRESULT rc = FR_DISK_ERR;
	FIL file_object;
	TCHAR buffer[128];

	if (hardware_get_mac_address(mac) != 0) {
		// There is no MAC-address
		memcpy(file_name, NO_MAC, sizeof(NO_MAC));
	} else {
		sprintf(file_name, "%02x%02x%02x%02x", (unsigned int) mac[2], (unsigned int) mac[3], (unsigned int) mac[4], (unsigned int) mac[5]);
	}

	(void) memcpy(&file_name[8], EXT_UID, 4);

	file_name[12] = '\0';

	rc = f_open(&file_object, (TCHAR *)file_name, (BYTE) FA_READ);

	if (rc == FR_OK) {
		if (f_gets(buffer, (int) sizeof(buffer), &file_object) != NULL) {
			if (uuid_parse((char *)buffer, out) == 0) {
				have_uuid = true;
			}
		}
		(void) f_close(&file_object);
	} else {
		rc = f_open(&file_object, (TCHAR *)file_name, (BYTE) (FA_WRITE | FA_CREATE_ALWAYS));
		if (rc == FR_OK) {
			uuid_generate_random(out);
			uuid_unparse(out, (char *)buffer);
			if (f_puts(buffer, &file_object) != EOF) {
				have_uuid = true;
			}
			(void) f_close(&file_object);
		}
	}

	if (!have_uuid) {
		(void) uuid_parse(DUMMY_UUID, out);
	}

	return have_uuid;
}
