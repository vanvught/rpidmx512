/**
 * @file json_get_directory.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstdio>
#include <cstdint>
#include <cassert>
#include <dirent.h>

#include "showfile.h"

namespace remoteconfig {
namespace showfile {

uint32_t json_get_directory(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nBufferSize = nOutBufferSize - 2U;
	auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nBufferSize, "{\"shows\":["));

	for (uint32_t nShowIndex = 0; nShowIndex < ShowFile::Get()->GetShows(); nShowIndex++) {
		const auto nShow = ShowFile::Get()->GetPlayerShowFile(nShowIndex);
		if (nShow >= 0) {
			uint32_t nFileSize;
			if (ShowFile::Get()->GetShowFileSize(static_cast<uint32_t>(nShow), nFileSize)) {
				const auto nSize = nBufferSize - nLength;
				const auto nCharacters = static_cast<uint32_t>(snprintf(&pOutBuffer[nLength], nSize, "{\"show\":%d,\"size\":%u},", nShow, nFileSize));

				if (nCharacters > nSize) {
					break;
				}

				nLength+=nCharacters;

				if (nLength >= nBufferSize) {
					break;
				}
			}
		}
	}

	if (pOutBuffer[nLength - 1] == ',') {
		nLength--;
	}

	pOutBuffer[nLength++] = ']';
	pOutBuffer[nLength++] = '}';

	assert(nLength <= nOutBufferSize);
	return nLength;
}
}  // namespace showfile
}  // namespace remoteconfig
