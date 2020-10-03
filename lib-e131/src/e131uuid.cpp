 /**
 * @file e131uuid.cpp
 *
 */
/* Copyright (C) 2019-2020 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <uuid/uuid.h>

#include "e131uuid.h"

#include "network.h"

#include "debug.h"

static constexpr char EXT_UID[] = ".uid";
static constexpr char DUMMY_UUID[] = "01234567-89ab-cdef-0134-567890abcedf";

E131Uuid::E131Uuid() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

E131Uuid::~E131Uuid() {
	DEBUG_ENTRY

	DEBUG_EXIT
}

void E131Uuid::GetHardwareUuid(uuid_t out) {
	bool bHaveUuid = false;
	char aFileName[16];	///< mac:8, .:1 ,uud:3 ,'\0':1
	uint8_t mac[8];
	FILE *fp;
	char aBuffer[128];

	Network::Get()->MacAddressCopyTo(mac);

	sprintf(aFileName, "%02x%02x%02x%02x", mac[2], mac[3], mac[4], mac[5]);

	memcpy(&aFileName[8], EXT_UID, 4);

	aFileName[12] = '\0';

	fp = fopen(aFileName, "r");
	if (fp != nullptr) {
		if (fgets(aBuffer, sizeof(aBuffer), fp) != nullptr) {
			if (uuid_parse(aBuffer, out) == 0) {
				bHaveUuid = true;
			}
		} else {
#if defined(__linux__) || defined (__CYGWIN__)
			perror("fgets");
#else
#ifndef NDEBUG
			printf("%s:%d\n", __FUNCTION__, __LINE__);
#endif
#endif
		}
		static_cast<void>(fclose(fp));
	} else {
		uuid_generate_random(out);
		uuid_unparse(out, aBuffer);
		bHaveUuid = true;

		fp = fopen(aFileName, "w+");
		if (fp != nullptr) {
			static_cast<void>(fputs(aBuffer, fp));
			static_cast<void>(fclose(fp));
		} else {
#if defined(__linux__) || defined (__CYGWIN__)
			perror("fopen");
#else
#ifndef NDEBUG
			printf("%s:%d\n", __FUNCTION__, __LINE__);
#endif
#endif
		}
	}

	if (!bHaveUuid) {
		static_cast<void>(uuid_parse(DUMMY_UUID, out));
#ifndef NDEBUG
		printf("%s:%d\n", __FUNCTION__, __LINE__);
#endif
	}
}
