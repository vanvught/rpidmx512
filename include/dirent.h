/**
 * @file dirent.h
 *
 */
/* Copyright (C) 2020-2024 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef DIRENT_H_
#define DIRENT_H_

#include <stdio.h>

#if !defined (FF_DEFINED)
	typedef void *DIR;
#endif

enum {
	DT_UNKNOWN = 0,
	DT_FIFO = 1,
	DT_CHR = 2,
	DT_DIR = 4,
	DT_BLK = 6,
	DT_REG = 8,
	DT_LNK = 10,
	DT_SOCK = 12,
	DT_WHT = 14
};

/**
 * https://en.wikibooks.org/wiki/C_Programming/POSIX_Reference/dirent.h
 */

struct dirent {
#if 0
	ino_t d_ino;
	off_t d_off;
	unsigned short d_reclen;
#endif
	unsigned char d_type;
	char d_name[FILENAME_MAX];
};

typedef struct dirent dirent_t;

#ifdef __cplusplus
extern "C" {
#endif

extern DIR *opendir(const char *dirname);
extern struct dirent *readdir(DIR *dirp);
extern int closedir(DIR* dirp);

#ifdef __cplusplus
}
#endif



#endif /* DIRENT_H_ */
