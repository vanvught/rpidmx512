/**
 * @file read_config_file.c
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

#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "read_config_file.h"

#include "ff.h"

/**
 *
 * @param file_name
 * @param pfi
 * @return
 */
const bool read_config_file(const char *file_name, funcptr pfi) {
	FRESULT rc = FR_DISK_ERR;
	FIL file_object;

	assert(file_name != NULL);
	assert(pfi != NULL);

	rc = f_open(&file_object, (TCHAR *)file_name, (BYTE) FA_READ);

	if (rc == FR_OK) {
		TCHAR buffer[128];
		for (;;) {
			if (f_gets(buffer, (int) sizeof(buffer), &file_object) == NULL) {
				break; // Error or end of file
			}
			(void) pfi((const char *) buffer);
		}
		(void) f_close(&file_object);
	} else {
		return false;
	}

	return true;
}
