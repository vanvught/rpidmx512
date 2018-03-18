/**
 * @file snprintf.c
 *
 */
/* Copyright (C) 2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <circle/string.h>
#include <circle/stdarg.h>
#include <circle/types.h>
#include <circle/util.h>

#include <assert.h>

extern "C" {

int snprintf(char *str, size_t size, const char *format, ...) {
	assert(str != 0);
	assert(format != 0);

	va_list var;
	va_start(var, format);

	CString SPrintf;
	SPrintf.FormatV(format, var);

	va_end(var);

	// char *strncpy(char *dest, const char *src, size_t n);
	// Warning: If there is no null byte among the first n bytes of src, the string placed in dest will not be null-terminated.
	strncpy(str, (const char *) SPrintf, size);

	if (size < SPrintf.GetLength()) {
		str[size - 1] = '\0';
	}

	return strlen(str);
}

}
