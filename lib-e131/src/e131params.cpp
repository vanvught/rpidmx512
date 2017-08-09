/**
 * @file e131params.cpp
 *
 */
/* Copyright (C) 2016 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <uuid/uuid.h>

#if defined(__linux__) || defined (__CYGWIN__)
#define ALIGNED
#include <string.h>
#else
#include "util.h"
#endif

#include "read_config_file.h"
#include "sscan.h"

#include "e131.h"
#include "e131params.h"

#define UUID_STRING_LENGTH	36

static const char PARAMS_FILE_NAME[] ALIGNED = "e131.txt";
static const char PARAMS_UNIVERSE[] ALIGNED = "universe";
static const char PARAMS_MERGE_MODE[] ALIGNED = "merge_mode";
static const char PARAMS_OUTPUT[] ALIGNED = "output";
static const char PARAMS_CID[] ALIGNED = "cid";

static uint16_t E131ParamsUniverse ALIGNED;
static _output_type E131ParamsOutputType ALIGNED;
static TMerge E131ParamsMergeMode;
static char E131ParamsCidString[UUID_STRING_LENGTH + 2] ALIGNED;
static bool E131HaveCustomCid;

/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
	char value[UUID_STRING_LENGTH + 2];
	uint8_t len;

	uint16_t value16;

	if (sscan_uint16_t(line, PARAMS_UNIVERSE, &value16) == 2) {
		if (value16 == 0 || value16 > E131_UNIVERSE_MAX) {
			E131ParamsUniverse = E131_UNIVERSE_DEFAULT;
		} else {
			E131ParamsUniverse = value16;
		}
		return;
	}

	len = 3;
	if (sscan_char_p(line, PARAMS_OUTPUT, value, &len) == 2) {
		if(memcmp(value, "mon", 3) == 0) {
			E131ParamsOutputType = OUTPUT_TYPE_MONITOR;
		}
		return;
	}

	len = 3;
	if (sscan_char_p(line, PARAMS_MERGE_MODE, value, &len) == 2) {
		if(memcmp(value, "ltp", 3) == 0) {
			E131ParamsMergeMode = E131_MERGE_LTP;
		}
		return;
	}

	len = UUID_STRING_LENGTH;
	if (sscan_uuid(line, PARAMS_CID, value, &len) == 2) {
		memcpy(E131ParamsCidString, value, UUID_STRING_LENGTH);
		E131ParamsCidString[UUID_STRING_LENGTH] = '\0';
		E131HaveCustomCid = true;
	}

}

/**
 *
 */
E131Params::E131Params(void) {
	E131ParamsUniverse = E131_UNIVERSE_DEFAULT;
	E131ParamsMergeMode = E131_MERGE_HTP;					///<
	E131ParamsOutputType = OUTPUT_TYPE_DMX;
	memset(E131ParamsCidString, 0, sizeof(E131ParamsCidString));
	E131HaveCustomCid = false;
}

/**
 *
 */
E131Params::~E131Params(void) {
}

/**
 *
 * @return
 */
bool E131Params::Load(void) {
	return read_config_file(PARAMS_FILE_NAME, &process_line_read);
}

/**
 *
 * @return range 1 to 63999
 */
const uint16_t E131Params::GetUniverse(void) {
	return E131ParamsUniverse;
}

/**
 *
 * @return \ref _output_type
 */
const _output_type E131Params::GetOutputType(void) {
	return E131ParamsOutputType;
}

/**
 *
 * @return
 */
const TMerge E131Params::GetMergeMode(void) {
	return E131ParamsMergeMode;
}

/**
 *
 * @return
 */
const bool E131Params::isHaveCustomCid(void) {
	return E131HaveCustomCid;
}

/**
 *
 * @return
 */
const char* E131Params::GetCidString(void) {
	return E131ParamsCidString;
}
