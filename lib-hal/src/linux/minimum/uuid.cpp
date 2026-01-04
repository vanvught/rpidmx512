/**
 * @file uuid.cpp
 *
 */
/* Copyright (C) 2024 by Arjan van Vught mailto:info@gd32-dmx.org
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of thnDmxDataDirecte Software, and to permit persons to whom the Software is
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
#include <cstring>
#include <uuid/uuid.h>

#include "exec_cmd.h"

namespace hal {
static constexpr auto UUID_STRING_LENGTH = 36;

void UuidCopy(uuid_t out) {
	char uuid_str[hal::UUID_STRING_LENGTH + 2];

#if defined (__APPLE__)
	constexpr char cmd[] = "sysctl -n kern.uuid";
#else
	constexpr char cmd[] = "cat /etc/machine-id";
#endif
	exec_cmd(cmd, uuid_str, sizeof(uuid_str));

#if defined (__APPLE__)
#else
	for (uint32_t i = 13; i > 0; i--) {
		uuid_str[36 - 13 + i] = uuid_str[36 - 13 + i - 4];
	}

	for (uint32_t i = 5; i > 0; i--) {
		uuid_str[23 - 5 + i] = uuid_str[23 - 5 + i - 3];
	}

	for (uint32_t i = 5; i > 0; i--) {
		uuid_str[18 - 5 + i] = uuid_str[18 - 5 + i - 2];
	}

	for (uint32_t i = 5; i > 0; i--) {
		uuid_str[13 - 5 + i] = uuid_str[13 - 5 + i - 1];
	}

	uuid_str[23] = '-';
	uuid_str[18] = '-';
	uuid_str[13] = '-';
	uuid_str[8] = '-';
#endif

	uuid_parse(uuid_str, out);
}
}  // namespace hal
