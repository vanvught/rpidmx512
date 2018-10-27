/**
 * @file stdio.h
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#ifndef STDIO_H_
#define STDIO_H_

#include <stddef.h>
#include <stdarg.h>

#define EOF	-1

typedef void* FILE;

#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern int puts(const char *s);

extern FILE *fopen(const char *path, const char *mode);
extern int fclose(FILE *stream);

extern int fgetc(FILE *stream);

extern char *fgets(char *s, int size, FILE *stream);
extern int fputs(const char *s, FILE *stream);

extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern int fseek(FILE *stream, long offset, int whence);

extern void clearerr(FILE *stream);
extern int ferror(FILE *stream);

extern int printf(const char *format, ...);

extern int sprintf(char *str, const char *format, ...);
extern int snprintf(char *str, size_t size, const char *format, ...);

extern int vprintf(const char *format, va_list ap);
extern int vsnprintf(char *str, size_t size, const char *format, va_list);

#ifdef __cplusplus
}
#endif

#endif /* STDIO_H_ */
