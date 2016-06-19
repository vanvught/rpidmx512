/**
 * @file oscutil.cpp
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

#ifdef __circle__

#include <circle/logger.h>
#include <circle/stdarg.h>
#include <assert.h>

#include "oscutil.h"

void *calloc (size_t nmemb, size_t size)
{
	size_t nbytes = nmemb * size;
	if (nbytes == 0)
	{
		return 0;
	}

	void *blk = malloc (nbytes);
	assert (blk != 0);

	memset (blk, 0, nbytes);
	
	return blk;
}

void *realloc (void *ptr, size_t size)		// TODO: inefficient
{
	//assert (ptr != 0); //->  If ptr is NULL, then the call is equivalent to malloc(size)

	if (ptr == 0)
	{
		void *newblk = malloc(size);
		assert(newblk != 0);
		return newblk;
	}

	if (size == 0)
	{
		free (ptr);

		return 0;
	}

	void *newblk = malloc (size);
	assert (newblk != 0);

	memcpy (newblk, ptr, size);

	free (ptr);

	return newblk;
}

void printf (const char *fmt, ...)
{
	assert (fmt != 0);

	// remove trailing '\n'
	size_t fmtlen = strlen (fmt);
	char fmtbuf[fmtlen+1];

	strcpy (fmtbuf, fmt);

	if (fmtbuf[fmtlen] == '\n')
	{
		fmtbuf[fmtlen] = '\0';
	}
	
	va_list var;
	va_start (var, fmt);

	CLogger::Get ()->WriteV ("osc", LogDebug, fmtbuf, var);

	va_end (var);
}
#endif
