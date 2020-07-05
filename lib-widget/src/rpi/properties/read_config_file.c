/**
 * @file read_config_file.c
 */
/* Copyright (C) 2017-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include <c/read_config_file.h>

bool read_config_file(const char *file_name, funcptr pfi) {
	char buffer[128];
	unsigned i;
	FILE *fp;

	assert(file_name != NULL);
	assert(pfi != NULL);

	fp = fopen(file_name, "r");

	if (fp != NULL) {
		for (;;) {
			if (fgets(buffer, (int) sizeof(buffer) - 1, fp) != buffer) {
				break; /* Error or end of file */
			}

			if (buffer[0] >= 'a') {
				char *q = (char*) buffer;

				for (i = 0; (i < sizeof(buffer) - 1) && (*q != '\0'); i++) {
					if ((*q == '\r') || (*q == '\n')) {
						*q = '\0';
					}
					q++;
				}

				pfi((const char*) buffer);
			}
		}

		fclose(fp);
	} else {
		return false;
	}

	return true;
}
