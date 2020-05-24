/**
 * @file perror.c
 *
 */
/* Copyright (C) 2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stddef.h>
#include <errno.h>

extern void console_error(const char *);
extern int console_putc(int);
extern int console_puts(const char *);

/*
errno -l | cut -f3- -d ' ' | sort -V -u | awk '$0="\""$0"\","'
*/

const char * const sys_errlist[] = {
 "OK",
 "Operation not permitted",
 "No such file or directory",
 "No such process",
 "Interrupted system call",
 "Input/output error",
 "No such device or address",
 "Argument list too long",
 "Exec format error",
 "Bad file descriptor",
 "No child processes",
 "Resource temporarily unavailable",
 "Cannot allocate memory",
 "Permission denied",
 "Bad address",
 "Block device required",
 "Device or resource busy",
 "File exists",
 "Invalid cross-device link",
 "No such device",
 "Not a directory",
 "Is a directory",
 "Invalid argument",
 "Too many open files in system",
 "Too many open files",
 "Inappropriate ioctl for device",
 "Text file busy",
 "File too large",
 "No space left on device",
 "Illegal seek",
 "Read-only file system",
 "Too many links",
 "Broken pipe",
 "Numerical argument out of domain",
 "Numerical result out of range",
 "Resource deadlock avoided",
 "File name too long",
 "No locks available",
 "Function not implemented",
 "Directory not empty",
 "Bad message"
};

char *strerror(int errnum) {
	if (errnum <= ELAST) {
		return (char *)sys_errlist[errnum];
	}

	return (char *)sys_errlist[EBADMSG];
}

void perror(const char *s) {
	const char *ptr = NULL;

	if (errno >= 0 && errno < ELAST) {
		ptr = sys_errlist[errno];
	} else {
		ptr = sys_errlist[EBADMSG];
	}

	if (s && *s) {
		console_error(s);
		console_puts(": ");
	}

	console_error(ptr);
	console_putc('\n');
}
