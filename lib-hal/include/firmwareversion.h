/**
 * @file firmwareversion.h
 *
 */
/* Copyright (C) 2019-2023 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#ifndef FIRMWAREVERSION_H_
#define FIRMWAREVERSION_H_

#include <cstdio>

#if !defined(STR_HELPER)
# define STR_HELPER(x) #x
# define STR(x) STR_HELPER(x)
#endif

namespace firmwareversion {
namespace length {
static constexpr auto SOFTWARE_VERSION = 3;
static constexpr auto GCC_DATE = 11;
static constexpr auto GCC_TIME = 8;
}  // namespace length
struct Info {
	char SoftwareVersion[length::SOFTWARE_VERSION];
	char BuildDate[length::GCC_DATE];
	char BuildTime[length::GCC_TIME];
};
}  // namespace firmwareversion

class FirmwareVersion {
public:
	FirmwareVersion(const char *pVersion, const char *pDate, const char *pTime);

	void Print(const char *pTitle = nullptr) {
		puts(s_Print);

		if (pTitle != nullptr) {
			printf("\x1b[32m%s\x1b[0m\n", pTitle);
		}
	}

	const struct firmwareversion::Info *GetVersion() {
		return &s_FirmwareVersion;
	}

	const char *GetPrint() {
		return s_Print;
	}

	const char *GetSoftwareVersion() {
		return s_FirmwareVersion.SoftwareVersion;
	}

	static FirmwareVersion *Get() {
		return s_pThis;
	}

private:
	static firmwareversion::Info s_FirmwareVersion;
	static char s_Print[64];
	static FirmwareVersion *s_pThis;
};

#endif /* FIRMWAREVERSION_H_ */
