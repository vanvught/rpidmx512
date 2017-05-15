/**
 * @file oscparams.cpp
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

#include "osc.h"
#include "oscparams.h"

#include "read_config_file.h"
#include "sscan.h"
#include "util.h"

static const char PARAMS_FILE_NAME[] ALIGNED = "osc.txt";				///< Parameters file name
static const char PARAMS_INCOMING_PORT[] ALIGNED = "incoming_port";		///<
static const char PARAMS_OUTGOING_PORT[] ALIGNED = "outgoing_port";		///<

static uint16_t OSCParamsIncomingPort ALIGNED = OSC_DEFAULT_INCOMING_PORT;
static uint16_t OSCParamsOutgoingPort ALIGNED = OSC_DEFAULT_OUTGOING_PORT;


/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
	uint16_t value16;

	if (sscan_uint16_t(line, PARAMS_INCOMING_PORT, &value16) == 2) {
		OSCParamsIncomingPort = value16;
	} else if (sscan_uint16_t(line, PARAMS_OUTGOING_PORT, &value16) == 2) {
		OSCParamsOutgoingPort = value16;
	}
}

/**
 *
 */
OSCParams::OSCParams(void) {
	OSCParamsIncomingPort = OSC_DEFAULT_INCOMING_PORT;
	OSCParamsOutgoingPort = OSC_DEFAULT_OUTGOING_PORT;
}

/**
 *
 */
OSCParams::~OSCParams(void) {
}

/**
 *
 * @return
 */
bool OSCParams::Load(void) {
	return read_config_file(PARAMS_FILE_NAME, &process_line_read);
}

/**
 *
 * @return
 */
const uint16_t OSCParams::GetIncomingPort(void) {
	return OSCParamsIncomingPort;
}

/**
 *
 * @return
 */
const uint16_t OSCParams::GetOutgoingPort(void) {
	return OSCParamsOutgoingPort;
}
