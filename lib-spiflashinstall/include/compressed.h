/**
 * @file compressed.h
 *
 */
/* Copyright (C) 2019 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef COMPRESSED_H_
#define COMPRESSED_H_

/**
 * This is a temporarily class until everyone is using compressed firmware
 */

#include <stdint.h>

enum TCheckCodes {
	CHECK_CODE_OK = 0,
	CHECK_CODE_CHECK_ERROR = (1 << 0),
	CHECK_CODE_UIMAGE_TOO_BIG = (1 << 1),
	CHECK_CODE_INVALID_HEADER = (1 << 2),
	CHECK_CODE_SPI_UPDATE_NEEDED = (1 << 3),
	CHECK_CODE_UIMAGE_COMPRESSED = (1 << 4)
};

class Compressed {
public:
	static uint32_t Check(const char *pFilename);
	static bool IsSupported();

private:
	static int32_t GetFileSize(const char *pFilename);
	static char *Find(const char *pBuffer, uint32_t nBufferLength, const char *pFind, uint32_t nFindLength);
};

#endif /* COMPRESSED_H_ */
