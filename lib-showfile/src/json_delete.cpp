/**
 * @file json_delete.cpp
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

#include <cstdint>
#include <cstdio>
#include <cassert>

#include "showfile.h"
#include "showfileconst.h"
#include "showfileparamsconst.h"

#include "sscan.h"

#include "debug.h"

namespace remoteconfig {
namespace showfile {
void json_delete(const char *pBuffer, const uint32_t nBufferSize) {
	DEBUG_ENTRY

	if (nBufferSize < sizeof("show=x")) {
		DEBUG_EXIT
		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(pBuffer, ShowFileParamsConst::SHOW, nValue8) == Sscan::OK) {
		if (nValue8 <= ::showfile::FILE_MAX_NUMBER) {
			ShowFile::Get()->DeleteShowFile(nValue8);

			DEBUG_EXIT
			return;
		}

		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}
}  // namespace showfile
}  // namespace remoteconfig
