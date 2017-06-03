/**
 * @file mode_0.c
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
#include <stdbool.h>
#include <stdio.h>

#include "tables.h"
#include "console.h"
#include "bridge_params.h"
#include "midi.h"
#include "dmx.h"
#include "monitor.h"

static const struct _midi_message *midi_message;			///<
static uint8_t midi_channel = (uint8_t) MIDI_CHANNEL_OMNI;	///<
static uint16_t dmx_start_address = (uint16_t) 1;			///<
static uint16_t dmx_max_slot = (uint16_t) DMX_UNIVERSE_SIZE;///<
static uint8_t dmx_data[DMX_DATA_BUFFER_SIZE] ALIGNED;		///<
static bool midi_active_sense_failed = false;				///<

static void clear_dmx_data(void) {
	uint32_t i = sizeof(dmx_data) / sizeof(dmx_data[0]) / sizeof(uint32_t);
	uint32_t *p = (uint32_t *)dmx_data;

	while (i-- != (uint32_t) 0) {
		*p++ = (uint32_t) 0;
	}
}

/**
 *
 */
void mode_0(void) {
	uint8_t dmx_index = (uint8_t) 0;
	bool dmx_new_data = false;

	if (midi_active_get_sense_state() == MIDI_ACTIVE_SENSE_FAILED) {
		if (!midi_active_sense_failed) {
			dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);
			midi_active_sense_failed = true;
		}

	} else if (midi_active_sense_failed) {
		dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, true);
		midi_active_sense_failed = false;
	}

	if (midi_read_channel(midi_channel)) {

		if (midi_message->channel != 0 ) {
			// Channel messages
			switch (midi_message->type) {
			case MIDI_TYPES_NOTE_OFF:
				dmx_index = dmx_start_address + midi_message->data1; // MIDI Channel starts with 0, DMX address starts with 1
				if (dmx_index <= DMX_UNIVERSE_SIZE) {	// we skip SC
					dmx_data[dmx_index] = (uint8_t) 0;
				}
				dmx_new_data = true;
				break;
			case MIDI_TYPES_NOTE_ON:
				dmx_index = dmx_start_address + midi_message->data1;
				if (dmx_index <= DMX_UNIVERSE_SIZE) {	// we skip SC
					uint8_t data = midi_message->data2 << 1;
					dmx_data[dmx_index] = data;
				}
				dmx_new_data = true;
				break;
			case MIDI_TYPES_CONTROL_CHANGE: {
				switch (midi_message->data1) {
					case MIDI_CONTROL_CHANGE_ALL_NOTES_OFF:
						clear_dmx_data();
						dmx_new_data = true;
						break;
					default:
						break;
				}
				break;
			}
			default:
				break;
			}

			if (dmx_new_data) {
				dmx_set_send_data(dmx_data, 1 + dmx_max_slot);
			}
		}
	}
}

INITIALIZER(modes, mode_0)

/**
 *
 */
void mode_0_monitor(void) {

}

INITIALIZER(modes_monitor, mode_0_monitor)

/**
 *
 */
void mode_0_init(void) {
	clear_dmx_data();

	midi_message = (const struct _midi_message *) midi_message_get();
	midi_channel = bridge_params_get_midi_channel();

	midi_active_sense_failed = (midi_active_get_sense_state() == MIDI_ACTIVE_SENSE_FAILED);

	dmx_start_address = bridge_params_get_dmx_start_address();
	dmx_max_slot = (dmx_start_address + (uint16_t) 127) <= DMX_UNIVERSE_SIZE ? (dmx_start_address + (uint16_t) 127) : DMX_UNIVERSE_SIZE + (uint16_t) 1; // SC

	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, false);
	dmx_set_send_data(dmx_data, 1 + dmx_max_slot);	// SC + data
	dmx_set_port_direction(DMX_PORT_DIRECTION_OUTP, true);

	monitor_line(5, "Listening channel : %d %s", midi_channel, midi_channel == 0 ? "<OMNI>" : "");
	monitor_line(6, "DMX start address : %d", dmx_start_address);
	monitor_line(7, "DMX slots         : %d", dmx_max_slot);
}

INITIALIZER(modes_init, mode_0_init)
