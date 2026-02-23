/**
 * @file dmxserialstatic.cpp
 *
 */
/* Copyright (C) 2020-2025 by Arjan van Vught mailto:info@gd32-dmx.org
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

#include <cstring>
#include <cctype>
#include <cstdint>
#include <cassert>

#include "dmxserial.h"

 #include "firmware/debug/debug_debug.h"

bool DmxSerial::FileNameCopyTo(char *file_name, uint32_t length, int32_t file_number) {
	assert(length == DmxSerialFile::NAME_LENGTH + 1);

	if ((file_number >= DmxSerialFile::MIN_NUMBER) && (file_number <= static_cast<int32_t>(DmxSerialFile::MAX_NUMBER))) {
		snprintf(file_name, length, DMXSERIAL_FILE_PREFIX "%.3d" DMXSERIAL_FILE_SUFFIX, file_number);
		return true;
	}

	return false;
}

bool DmxSerial::CheckFileName(const char *file_name, int32_t &file_number) {
	DEBUG_PRINTF("file_name=[%s]", file_name);

	if ((file_name == nullptr) || (strlen(file_name) != DmxSerialFile::NAME_LENGTH)) {
		DEBUG_EXIT();
		return false;
	}

	if (memcmp(file_name, DMXSERIAL_FILE_PREFIX, sizeof(DMXSERIAL_FILE_PREFIX) - 1) != 0) {
		DEBUG_EXIT();
		return false;
	}

	if (memcmp(&file_name[DmxSerialFile::NAME_LENGTH - sizeof(DMXSERIAL_FILE_SUFFIX) + 1], DMXSERIAL_FILE_SUFFIX, sizeof(DMXSERIAL_FILE_SUFFIX) - 1) != 0) {
		DEBUG_EXIT();
		return false;
	}

	char digit = file_name[sizeof(DMXSERIAL_FILE_PREFIX) - 1];

	if (!isdigit(digit)) {
		DEBUG_EXIT();
		return false;
	}

	file_number = 100 * (digit - '0');

	if (file_number > static_cast<int32_t>(DmxSerialFile::MAX_NUMBER)) {
		DEBUG_EXIT();
		return false;
	}

	digit = file_name[sizeof(DMXSERIAL_FILE_PREFIX)];

	if (!isdigit(digit)) {
		DEBUG_EXIT();
		return false;
	}

	file_number += (10 * (digit - '0'));

	digit = file_name[sizeof(DMXSERIAL_FILE_PREFIX) + 1];

	if (!isdigit(digit)) {
		DEBUG_EXIT();
		return false;
	}

	file_number += (digit - '0');

	if (file_number > static_cast<int32_t>(DmxSerialFile::MAX_NUMBER)) {
		DEBUG_EXIT();
		return false;
	}

	DEBUG_EXIT();
	return true;
}
