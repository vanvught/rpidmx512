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

#include "e131params.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "e131.txt";				///< Parameters file name
static const char PARAMS_UNIVERSE[] ALIGNED = "universe";				///<
static const char PARAMS_OUTPUT[] ALIGNED = "output";					///<

/**
 * 6.7 Universe
 * The Universe is a 16-bit field that defines the universe number of the data carried in the packet. Universe
 * values shall be limited to the range 1 to 63999. Universe value 0 and those between 64000 and 65535 are
 * reserved for future use. See Section 8 for more information.
 */
#define DEFAULT_UNIVERSE	1
#define MAX_UNIVERSE		63999

static uint16_t E131ParamsUniverse ALIGNED = DEFAULT_UNIVERSE; 			///<
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
		if (value16 == 0 || value16 > MAX_UNIVERSE) {
			E131ParamsUniverse = DEFAULT_UNIVERSE;
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
	E131ParamsUniverse = DEFAULT_UNIVERSE;
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
