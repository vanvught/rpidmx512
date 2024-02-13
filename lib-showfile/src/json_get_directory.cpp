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
#ifndef NDEBUG
# include <errno.h>
#endif

#include "showfile.h"

namespace remoteconfig {
namespace showfile {

uint32_t json_get_directory(char *pOutBuffer, const uint32_t nOutBufferSize) {
	const auto nBufferSize = nOutBufferSize - 2U;
#if defined (CONFIG_USB_HOST_MSC)
	auto *dirp = opendir("0:/");
#else
	auto *dirp = opendir(".");
#endif
#ifndef NDEBUG
	perror("opendir");
#endif

	auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nBufferSize, "{\"shows\":["));

	if (dirp != nullptr) {
		struct dirent *dp;
		do {
			if ((dp = readdir(dirp)) != nullptr) {
				if (dp->d_type == DT_DIR) {
					continue;
				}

	          	uint32_t nShowFileNumber;
	        	if (!::showfile::filename_check(dp->d_name, nShowFileNumber)) {
	                continue;
	            }

				const auto nSize = nBufferSize - nLength;
				const auto nCharacters = static_cast<uint32_t>(snprintf(&pOutBuffer[nLength], nSize, "\"%d\",", nShowFileNumber));

				if (nCharacters > nSize) {
					break;
				}

				nLength+=nCharacters;

				if (nLength >= nBufferSize) {
					break;
				}
			}
		} while (dp != nullptr);

		if (pOutBuffer[nLength - 1] == ',') {
			nLength--;
		}
	}

	pOutBuffer[nLength++] = ']';
	pOutBuffer[nLength++] = '}';

	assert(nLength <= nOutBufferSize);
	return nLength;
}
}  // namespace showfile
}  // namespace remoteconfig