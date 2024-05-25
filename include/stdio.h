/**
 * @file stdio.h
 *
 */
/* Copyright (C) 2017-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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
#include <stdint.h>

#define EOF	(-1)

typedef struct __sFILE {
	void *udata;
#define	__SLBF	0x0001		/* line buffered */
#define	__SNBF	0x0002		/* unbuffered */
#define	__SRD	0x0004		/* OK to read */
#define	__SWR	0x0008		/* OK to write */
	/* RD and WR are never simultaneously asserted */
#define	__SRW	0x0010		/* open for reading & writing */
#define	__SEOF	0x0020		/* found EOF */
#define	__SERR	0x0040		/* found error */
	uint8_t flags;
} FILE;

#if defined(CONFIG_POSIX_ENABLE_STDIN)
#error Not supported
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
#endif

#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

#define FILENAME_MAX	16

#ifdef __cplusplus
extern "C" {
#endif

int puts(const char *s);
int putchar(int c);

int fileno(FILE *stream);

FILE *fopen(const char *path, const char *mode);
int fclose(FILE *stream);

int fgetc(FILE *stream);
int fputc(int c, FILE *stream);

char *fgets(char *s, int size, FILE *stream);
int fputs(const char *s, FILE *stream);

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

int fseek(FILE *stream, long offset, int whence);

long ftell(FILE *stream);

void clearerr(FILE *stream);
int ferror(FILE *stream);
int feof(FILE *stream);

int printf(const char *format, ...);

int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);

int vprintf(const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list);

void perror(const char *s);

#ifdef __cplusplus
}
#endif

#endif /* STDIO_H_ */
