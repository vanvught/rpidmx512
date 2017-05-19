/**
 * @file bridge_params.c
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
#include <stdio.h>

#include "read_config_file.h"
#include "sscan.h"
#include "util.h"
#include "midi.h"
#include "dmx.h"
#include "bridge.h"
#include "bridge_params.h"
#include "bridge_monitor.h"
#include "tables.h"

TABLE(initializer_t, modes)
TABLE(initializer_t, modes_monitor)
TABLE(initializer_t, modes_init)

static const char PARAMS_FILE_NAME[] ALIGNED = "params.txt";			///< Parameters file name
static const char PARAMS_BREAK_TIME[] ALIGNED = "dmx_break_time";		///<
static const char PARAMS_MAB_TIME[] ALIGNED = "dmx_mab_time";			///<
static const char PARAMS_REFRESH_RATE[] ALIGNED = "dmx_refresh_rate";	///<
static const char PARAMS_START_ADDRESS[] ALIGNED = "dmx_start_address";	///<
static const char PARAMS_CHANNEL[] ALIGNED = "midi_channel";			///<
static const char PARAMS_MODE[] ALIGNED = "bridge_mode";				///<

static uint8_t bridge_params_break_time = BRIDGE_PARAMS_DEFAULT_BREAK_TIME;		///< DMX output break time in 10.67 microsecond units. Valid range is 9 to 127.
static uint8_t bridge_params_mab_time = BRIDGE_PARAMS_DEFAULT_MAB_TIME;			///< DMX output Mark After Break time in 10.67 microsecond units. Valid range is 1 to 127.
static uint8_t bridge_params_refresh_rate = BRIDGE_PARAMS_DEFAULT_REFRESH_RATE;	///< DMX output rate in packets per second. Valid range is 1 to 40.
static uint16_t bridge_params_dmx_start_address = 1;							///<
static uint8_t bridge_params_midi_channel = MIDI_CHANNEL_OMNI;					///<
static uint8_t bridge_params_bridge_mode = 0;									///<
static uint8_t bridge_params_table_index = 0;									///<

/**
 *
 * @param line
 */
static void process_line_read(const char *line) {
	uint8_t value8;
	uint32_t value32;

	if (sscan_uint8_t(line, PARAMS_BREAK_TIME, &value8) == 2) {
		if ((value8 >= (uint8_t) BRIDGE_PARAMS_MIN_BREAK_TIME) && (value8 <= (uint8_t) BRIDGE_PARAMS_MAX_BREAK_TIME)) {
			bridge_params_break_time = value8;
		}
	} else if (sscan_uint8_t(line, PARAMS_MAB_TIME, &value8) == 2) {
		if ((value8 >= (uint8_t) BRIDGE_PARAMS_MIN_MAB_TIME) && (value8 <= (uint8_t) BRIDGE_PARAMS_MAX_MAB_TIME)) {
			bridge_params_mab_time = value8;
		}
	} else if (sscan_uint8_t(line, PARAMS_REFRESH_RATE, &value8) == 2) {
		bridge_params_refresh_rate = value8;
	}  else if (sscan_uint8_t(line, PARAMS_CHANNEL, &value8) == 2) {
		if (value8 > MIDI_CHANNEL_OFF) {
			bridge_params_midi_channel = (uint8_t) MIDI_CHANNEL_OFF;
		} else {
			bridge_params_midi_channel = value8;
		}
	} else if (sscan_uint8_t(line, PARAMS_MODE, &value8) == 2) {
		bridge_params_bridge_mode = value8;
	}

	if (sscan_uint32_t(line, PARAMS_START_ADDRESS, &value32) == 2) {
		if (bridge_params_dmx_start_address > DMX_UNIVERSE_SIZE) {
			bridge_params_dmx_start_address = (uint16_t) DMX_UNIVERSE_SIZE;
		} else {
			bridge_params_dmx_start_address = (uint16_t) value32;
		}
	}
}

/**
 *
 * @return
 */
const uint8_t bridge_params_get_break_time(void) {
	return bridge_params_break_time;
}

/**
 *
 * @return
 */
const uint8_t bridge_params_get_mab_time(void) {
	return bridge_params_mab_time;
}

/**
 *
 * @return
 */
const uint8_t bridge_params_get_refresh_rate(void) {
	return bridge_params_refresh_rate;
}

/**
 *
 * @return
 */
const uint16_t bridge_params_get_dmx_start_address(void) {
	return bridge_params_dmx_start_address;
}

/**
 *
 * @return
 */
const uint8_t bridge_params_get_midi_channel(void) {
	return bridge_params_midi_channel;
}

/**
 *
 * @return
 */
const uint8_t bridge_params_get_bridge_mode(void) {
	return bridge_params_bridge_mode;
}

/**
 *
 * @return
 */
const uint8_t bridge_params_get_table_index(void) {
	return bridge_params_table_index;
}

void bridge_params_init(void) {
	uint32_t period;
	int j;
	char mode_function_name[] = "mode_xxx";

	read_config_file(PARAMS_FILE_NAME, &process_line_read);

	period = 0;

	if (bridge_params_refresh_rate != 0) {
		period = (uint32_t) (1E6 / bridge_params_refresh_rate);
	}

	dmx_set_output_period(period);
	dmx_set_output_break_time((uint32_t) ((double) (bridge_params_break_time) * (double) (10.67)));
	dmx_set_output_mab_time((uint32_t) ((double) (bridge_params_mab_time) * (double) (10.67)));

	// It is expected that there is always a default 'mode_0.c' implementation.
	bridge_set_func(modes_table[0].f);
	bridge_monitor_set_func(modes_monitor_table[0].f);

	sprintf(mode_function_name, "mode_%d", (int)bridge_params_bridge_mode);

	for (j = 0; j < TABLE_LENGTH(modes); j++) {
		if (strcmp(modes_table[j].name, mode_function_name) == 0) {
			bridge_params_table_index = (uint8_t) j;
			bridge_set_func(modes_table[j].f);
			bridge_monitor_set_func(modes_monitor_table[j].f);
			break;
		}
	}
}
