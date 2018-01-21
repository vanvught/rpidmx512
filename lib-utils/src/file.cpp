/**
 * @file file.ccp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stddef.h>
#include <assert.h>

#include "ff.h"

#if defined (BARE_METAL)
#include "console.h"
#include "util.h"
#elif defined (__circle__)
#include "circle/util.h"
#endif

static FIL file_object;

FILE *fopen(const char *path, const char *mode) {
	BYTE fa = (BYTE) FA_READ;

	assert(path != NULL);
	assert(mode != NULL);

	if (strcmp(mode, "r") == 0) {
		fa = (BYTE) FA_READ;
	} else if (strcmp(mode, "w+") == 0) {
		fa = (BYTE) (FA_WRITE | FA_CREATE_ALWAYS);
	} else {
#if defined (BARE_METAL)
		(void) console_error("mode is not implemented");
		(void) console_error(mode);
#endif
		return NULL;
	}

	if (f_open(&file_object, (TCHAR *)path, fa) == FR_OK) {
		return (FILE *)&file_object;
	} else {
		return NULL;
	}
}

int fclose(FILE *stream) {
	if (stream == NULL) {
		return 0;
	}

	if (f_close(&file_object) == FR_OK) {
		return 0;
	} else {
		return -1;
	}
}

int fgetc(FILE *stream) {
	char c;
	UINT bytes_read;

	if (stream == NULL) {
		return EOF;
	}

	if (f_read(&file_object, &c, (UINT) 1, &bytes_read) == FR_OK) {
		if (bytes_read > 0) {
			return c;
		}

		if (bytes_read < 1) {
			return (EOF);
		}
	}

	return (EOF);
}

#if !defined (__circle__)
char *fgets(char *s, int size, FILE *stream) {
	if (stream == NULL) {
		*s = '\0';
		return NULL;
	}

	if (f_gets((TCHAR *) s, size, &file_object) != (TCHAR *)s) {
		*s = '\0';
		return NULL;
	}

	return s;
}

int fputs(const char *s, FILE *stream) {
	assert(s != NULL);

	if (stream == NULL) {
		return 0;
	}

	return f_puts((const TCHAR *) s, &file_object);
}
#endif
