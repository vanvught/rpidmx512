/**
 * @file envparams.cpp
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

#if !defined(__clang__)	// Needed for compiling on MacOS
# pragma GCC push_options
# pragma GCC optimize ("Os")
#endif

#include <cstdint>
#include <cstring>
#include <cassert>

#include "envparams.h"
#include "envparamsconst.h"

#include "readconfigfile.h"
#include "sscan.h"

#include "propertiesbuilder.h"

#include "configstore.h"

#include "debug.h"

EnvParams::EnvParams() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void EnvParams::LoadAndSet() {
	DEBUG_ENTRY

	assert(ConfigStore::Get() != nullptr);

#if !defined(DISABLE_FS)
	ReadConfigFile configfile(EnvParams::staticCallbackFunction, this);
	configfile.Read(EnvParamsConst::FILE_NAME);
#endif

#ifndef NDEBUG
	Dump();
#endif

	DEBUG_EXIT
}

void EnvParams::LoadAndSet(const char *pBuffer, uint32_t nLength) {
	DEBUG_ENTRY

	assert(ConfigStore::Get() != nullptr);

	assert(pBuffer != nullptr);
	assert(nLength != 0);

	ReadConfigFile config(EnvParams::staticCallbackFunction, this);
	config.Read(pBuffer, nLength);

#ifndef NDEBUG
	Dump();
#endif
	DEBUG_EXIT
}

void EnvParams::callbackFunction(const char *pLine) {
	assert(pLine != nullptr);

	int8_t nHours;
	uint8_t nMinutes;

	if (Sscan::UtcOffset(pLine, EnvParamsConst::UTC_OFFSET, nHours, nMinutes) == Sscan::OK) {
		ConfigStore::Get()->SetEnvUtcOffset(nHours, nMinutes);
		return;
	}
}

void EnvParams::staticCallbackFunction(void *p, const char *s) {
	assert(p != nullptr);
	assert(s != nullptr);

	(static_cast<EnvParams *>(p))->callbackFunction(s);
}

void EnvParams::Builder(char *pBuffer, uint32_t nLength, uint32_t& nSize) {
	DEBUG_ENTRY

	assert(pBuffer != nullptr);

	PropertiesBuilder builder(EnvParamsConst::FILE_NAME, pBuffer, nLength);

	int8_t nHours;
	uint8_t nMinutes;
	ConfigStore::Get()->GetEnvUtcOffset(nHours, nMinutes);
	builder.AddUtcOffset(EnvParamsConst::UTC_OFFSET, nHours, nMinutes);

	nSize = builder.GetSize();

	DEBUG_PRINTF("nSize=%d", nSize);
	DEBUG_EXIT
}

void EnvParams::Dump() {
	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, EnvParamsConst::FILE_NAME);

	puts("UTC Offset");
	int8_t nHours;
	uint8_t nMinutes;
	ConfigStore::Get()->GetEnvUtcOffset(nHours, nMinutes);
	printf(" %s=%.2d:%.2u [%d]\n", EnvParamsConst::UTC_OFFSET, nHours, nMinutes, ConfigStore::Get()->GetEnvUtcOffset());
}
