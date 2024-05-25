/**
 * @file json_rtc.cpp
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
#include <cstring>
#include <cassert>

#include "hwclock.h"

#include "debug.h"

// ISO 8601 format (YYYY-MM-DDTHH:MM:SS)

#include "readconfigfile.h"
#include "sscan.h"

static constexpr auto iso8601FormatSize = sizeof("YYYY-MM-DDTHH:MM:SS");
static char iso8601Format[iso8601FormatSize];

static void staticCallbackFunction([[maybe_unused]] void *p, const char *s) {
	assert(p == nullptr);
	assert(s != nullptr);

	uint32_t nLength = iso8601FormatSize;

	if (Sscan::Char(s, "alarm" , iso8601Format, nLength) == Sscan::OK) {
		DEBUG_PUTS("alarm");
		return;
	}

	char action[8];
	nLength = sizeof(action) - 1;

	if (Sscan::Char(s, "action" , action, nLength) == Sscan::OK) {
		if (strncmp(action, "hctosys", nLength) == 0) {
			HwClock::Get()->HcToSys();
			return;
		}

		if (strncmp(action, "systohc", nLength) == 0) {
			HwClock::Get()->SysToHc();
			return;
		}

		return;
	}

	uint8_t nValue8;

	if (Sscan::Uint8(s, "enable", nValue8) == Sscan::OK) {
		HwClock::Get()->AlarmEnable(nValue8 != 0);
		return;
	}
}

namespace remoteconfig {
namespace rtc {
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

uint32_t json_get_rtc(char *pOutBuffer, const uint32_t nOutBufferSize) {
	DEBUG_ENTRY

	struct tm tm;

	if (HwClock::Get()->Get(&tm)) {
			auto nLength = static_cast<uint32_t>(snprintf(pOutBuffer, nOutBufferSize,
				"{\"rtc\":\"%d-%.2d-%.2dT%.2d:%.2d:%.2d\",",
				1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
				tm.tm_hour, tm.tm_min, tm.tm_sec));

		if (HwClock::Get()->AlarmGet(&tm)) {
			nLength += static_cast<uint32_t>(snprintf(&pOutBuffer[nLength], nOutBufferSize,
					"\"alarm\":\"%d-%.2d-%.2dT%.2d:%.2d:%.2d\",",
					1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
					tm.tm_hour, tm.tm_min, tm.tm_sec));

			nLength += static_cast<uint32_t>(snprintf(&pOutBuffer[nLength], nOutBufferSize,
					"\"enabled\":\"%d\"}", HwClock::Get()->AlarmIsEnabled()));

			DEBUG_EXIT
			return nLength;
		} else {
			pOutBuffer[nLength++] = '}';
		}
	}

	DEBUG_EXIT
	return 0;
}

void json_set_rtc(const char *pBuffer, const uint32_t nBufferSize) {
	DEBUG_ENTRY
	debug_dump(pBuffer, static_cast<uint16_t>(nBufferSize));

	iso8601Format[0] = 0;

	ReadConfigFile config(staticCallbackFunction, nullptr);
	config.Read(pBuffer, nBufferSize);

	debug_dump(iso8601Format, static_cast<uint16_t>(sizeof(iso8601Format)));

	if (iso8601Format[0] != 0) {
		struct tm tm;
		tm.tm_year = atoi(&iso8601Format[0],  4) - 1900;
		tm.tm_mon  = atoi(&iso8601Format[5],  2) - 1;
		tm.tm_mday = atoi(&iso8601Format[8],  2);
		tm.tm_hour = atoi(&iso8601Format[11], 2);
		tm.tm_min  = atoi(&iso8601Format[14], 2);
		tm.tm_sec  = atoi(&iso8601Format[17], 2);
		HwClock::Get()->AlarmSet(&tm);

		DEBUG_EXIT
		return;
	}

	DEBUG_EXIT
}
}  // namespace rtc
}  // namespace remoteconfig
