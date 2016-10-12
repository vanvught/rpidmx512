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

#include "read_config_file.h"
#include "sscan.h"
#include "util.h"

#include "e131.h"
#include "e131params.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "e131.txt";				///< Parameters file name
static const char PARAMS_UNIVERSE[] ALIGNED = "universe";				///<
static const char PARAMS_OUTPUT[] ALIGNED = "output";					///<

static uint16_t E131ParamsUniverse ALIGNED = E131_UNIVERSE_DEFAULT;	///<
static _output_type E131ParamsOutputType ALIGNED = OUTPUT_TYPE_DMX;	///<

/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
	char value[8] ALIGNED;
	uint8_t len = 3;

	uint16_t value16;

	if (sscan_uint16_t(line, PARAMS_UNIVERSE, &value16) == 2) {
		if (value16 == 0 || value16 > E131_UNIVERSE_MAX) {
			E131ParamsUniverse = E131_UNIVERSE_DEFAULT;
		} else {
			E131ParamsUniverse = value16;
		}
	} else if (sscan_char_p(line, PARAMS_OUTPUT, value, &len) == 2) {
		if(memcmp(value, "mon", 3) == 0) {
			E131ParamsOutputType = OUTPUT_TYPE_MONITOR;
		}
	}
}

/**
 *
 */
E131Params::E131Params(void) {
	E131ParamsUniverse = E131_UNIVERSE_DEFAULT;
	E131ParamsOutputType = OUTPUT_TYPE_DMX;
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
