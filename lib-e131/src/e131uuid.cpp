 /**
 * @file e131uuid.cpp
 *
 */
/* Copyright (C) 2017-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stdbool.h>
#include <stdio.h>
#include <uuid/uuid.h>

#include "e131uuid.h"

#include "network.h"

#if defined(BARE_METAL)
 #include "util.h"
#else
 #include <stddef.h>
 #include <string.h>
#endif

#ifndef ALIGNED
 #define ALIGNED __attribute__ ((aligned (4)))
#endif

static const char EXT_UID[] ALIGNED = ".uid";
static const char DUMMY_UUID[] ALIGNED = "01234567-89ab-cdef-0134-567890abcedf";

E131Uuid::E131Uuid(void) : m_bHaveUuid(false) {
}

E131Uuid::~E131Uuid(void) {
	m_bHaveUuid = false;
}

bool E131Uuid::GetHardwareUuid(uuid_t out) {
	char file_name[16];	///< mac:8, .:1 ,uud:3 ,'\0':1
	uint8_t mac[8];
	FILE *fp;
	char buffer[128];

	Network::Get()->MacAddressCopyTo(mac);

	sprintf(file_name, "%02x%02x%02x%02x", (unsigned int) mac[2], (unsigned int) mac[3], (unsigned int) mac[4], (unsigned int) mac[5]);

	(void) memcpy(&file_name[8], EXT_UID, 4);

	file_name[12] = '\0';

	fp = fopen(file_name, "r");
	if (fp != NULL) {
		if (fgets(buffer, (int) sizeof(buffer), fp) != NULL) {
			if (uuid_parse((char *)buffer, out) == 0) {
				m_bHaveUuid = true;
			}
		} else {
#if defined(__linux__) || defined (__CYGWIN__)
			perror("fgets");
#endif
		}
		(void) fclose(fp);
	} else {
		fp = fopen(file_name, "w+");
		if (fp != NULL) {
			uuid_generate_random(out);
			uuid_unparse(out, (char *)buffer);
			if (fputs(buffer, fp) > 0) {
				m_bHaveUuid = true;
			}
			(void) fclose(fp);
		} else {
#if defined(__linux__) || defined (__CYGWIN__)
			perror("fopen");
#endif
		}
	}

	if (!m_bHaveUuid) {
		(void) uuid_parse(DUMMY_UUID, out);
	}

	return m_bHaveUuid;
}
