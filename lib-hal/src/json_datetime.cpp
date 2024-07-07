/**
 * @file json_datetime.cpp
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
#include <time.h>
#include <sys/time.h>
#include <cassert>

#include "configstore.h"

#include "debug.h"

namespace remoteconfig {
namespace timedate {
static int atoi(const char *pBuffer, uint32_t nSize) {
	assert(pBuffer != nullptr);
	assert(nSize <= 4);

	const char *p = pBuffer;
	int32_t res = 0;

	for (; (nSize > 0) && (*p >= '0' && *p <= '9'); nSize--) {
		res = res * 10 + *p - '0';
		p++;
	}

	return res;
}

uint32_t json_get_timeofday(char *pOutBuffer, const uint32_t nOutBufferSize) {
	DEBUG_ENTRY

	struct timeval tv;
	if (gettimeofday(&tv, nullptr) >= 0) {
		auto *tm = localtime(&tv.tv_sec);

		int8_t nHours;
		uint8_t nMinutes;
		ConfigStore::Get()->GetEnvUtcOffset(nHours, nMinutes);

		if ((nHours == 0) && (nMinutes == 0)) {
			const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
					"{\"date\":\"%d-%.2d-%.2dT%.2d:%.2d:%.2dZ\"}\n",
					1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec));

			DEBUG_EXIT
			return nLength;
		} else {
			const auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
					"{\"date\":\"%d-%.2d-%.2dT%.2d:%.2d:%.2d%s%.2d:%.2u\"}\n",
					1900 + tm->tm_year, 1 + tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
					nHours > 0 ? "+" : "", nHours, nMinutes));

			DEBUG_EXIT
			return nLength;
		}
	}

	DEBUG_EXIT
	return 0;
}

void json_set_timeofday(const char *pBuffer, const uint32_t nBufferSize) {
	DEBUG_ENTRY
	debug_dump(pBuffer, nBufferSize);

	if ((nBufferSize == 26) || (nBufferSize == 31)) {
		struct tm tm;
		tm.tm_year = atoi(&pBuffer[5], 4) - 1900;
		tm.tm_mon = atoi(&pBuffer[10], 2) - 1;
		tm.tm_mday = atoi(&pBuffer[13], 2);
		tm.tm_hour = atoi(&pBuffer[16], 2);
		tm.tm_min = atoi(&pBuffer[19], 2);
		tm.tm_sec = atoi(&pBuffer[22], 2);

		struct timeval tv;
		tv.tv_sec = mktime(&tm);
		tv.tv_usec = 0;

		if (nBufferSize == 26) {
			assert(pBuffer[23] == 'Z');
		} else {
			const int8_t nSign = pBuffer[24] == '-' ? -1 : 1;
			const auto nHours = static_cast<int8_t>(atoi(&pBuffer[25], 2) * nSign);
			const auto nMinutes = static_cast<uint8_t>(atoi(&pBuffer[28], 2));

			ConfigStore::Get()->SetEnvUtcOffset(nHours, nMinutes);

			tv.tv_sec = tv.tv_sec - ConfigStore::Get()->GetEnvUtcOffset();
		}

		settimeofday(&tv, nullptr);

		DEBUG_PRINTF("%.4d/%.2d/%.2d %.2d:%.2d:%.2d", 1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}
}  // namespace timedate
}  // namespace remoteconfig
