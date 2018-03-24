/**
 * @file main.c
 *
 */
/* Copyright (C) 2016-2018 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include <stdio.h>

#include "hardwarebaremetal.h"
#include "networkbaremetal.h"

#include "console.h"

#include "lcd.h"
#include "display_oled.h"
#include "display_7segment.h"
#include "display_matrix.h"

#include "midi_sender.h"
#include "midi_reader.h"

#include "ltc_reader.h"
#include "ltc_reader_params.h"

#include "artnetnode.h"
#include "artnetreader.h"
#include "artnet_output.h"

#include "wifi.h"
#include "wifi_udp.h"

#include "software_version.h"

static struct _ltc_reader_output output = { true, false, false, false, false, false, false };

extern "C" {

static void handle_bool(const bool b) {
	if (b) {
		console_save_color();
		console_set_fg_color(CONSOLE_GREEN);
		(void) console_puts("Yes");
		console_restore_color();
	} else {
		(void) console_puts("No");
	}
}

void notmain(void) {
	HardwareBaremetal hw;
	NetworkBaremetal nw;
	uint8_t nHwTextLength;
	struct ip_info ip_config;
	ltc_reader_source_t source = LTC_READER_SOURCE_LTC;
	ArtNetNode node;
	ArtNetReader reader;

	ltc_reader_params_init();

	source = ltc_reader_params_get_source();

	output.console_output = ltc_reader_params_is_console_output();
	output.lcd_output = ltc_reader_params_is_lcd_output();
	output.oled_output = ltc_reader_params_is_oled_output();
	output.segment_output = ltc_reader_params_is_7segment_output();
	output.midi_output = ltc_reader_params_is_midi_output();
	output.artnet_output = ltc_reader_params_is_artnet_output();
	output.matrix_output = ltc_reader_params_is_matrix_output();

	printf("[V%s] %s Compiled on %s at %s\n", SOFTWARE_VERSION, hw.GetBoardName(nHwTextLength), __DATE__, __TIME__);
	printf("SMPTE TimeCode LTC Reader / Protocol converter");

	hw.SetLed(HARDWARE_LED_ON);

	console_set_top_row(3);

	switch (source) {
		case LTC_READER_SOURCE_ARTNET:
			output.artnet_output = false;
			break;
		case LTC_READER_SOURCE_MIDI:
			output.midi_output = false;
			break;
		default:
			break;
	}

	if (output.artnet_output || (source == LTC_READER_SOURCE_ARTNET) ) {
		if (wifi(&ip_config)) {
			console_status(CONSOLE_YELLOW, "Starting UDP ...");
			wifi_udp_begin(6454);

			console_status(CONSOLE_YELLOW, "Setting Node parameters ...");

			node.SetOutput(NULL);
			node.SetDirectUpdate(false);

			node.SetShortName("AvV Art-Net Node");
			node.SetLongName("Raspberry Pi Art-Net 3 Node TimeCode");

			console_status(CONSOLE_YELLOW, "Starting Node ...");

			node.Start();

			console_status(CONSOLE_GREEN, "ArtNet started");
		} else {
			output.artnet_output = false;
			if (source == LTC_READER_SOURCE_ARTNET) {
				source = LTC_READER_SOURCE_LTC;
			}
		}
	}

	if(output.midi_output) {
		midi_sender_init();
	}

	if (output.lcd_output) {
		output.lcd_output = lcd_detect();
	}

	if (output.oled_output) {
		output.oled_output = display_oled_init();
	}

	if (output.segment_output) {
		display_7segment_init(ltc_reader_params_get_max7219_intensity());
	}

	if (output.artnet_output) {
		artnet_output_set_node(&node);
	}

	if (output.matrix_output) {
		display_matrix_init(ltc_reader_params_get_max7219_intensity());
	}

	console_set_cursor(0, 15);
	(void) console_puts("Source : ");

	switch (source) {
	case LTC_READER_SOURCE_ARTNET:
		(void) console_puts("Art-Net");
		node.SetTimeCodeHandler(&reader);
		reader.Start(&output);
		break;
	case LTC_READER_SOURCE_MIDI:
		(void) console_puts("MIDI");
		midi_reader_init(&output);
		printf(", baudrate : %d, interface : %s", (int) midi_get_baudrate(), midi_get_interface_description());
		break;
	default:
		ltc_reader_init(&output);
		(void) console_puts("LTC");
		break;
	}

	(void) console_puts("\n\nConsole output   : ");
	handle_bool(output.console_output);
	(void) console_puts("\nLCD output       : ");
	handle_bool(output.lcd_output);
	(void) console_puts("\nOLED output      : ");
	handle_bool(output.oled_output);
	(void) console_puts("\n7-Segment output : ");
	handle_bool(output.segment_output);
	(void) console_puts("\n8-Matrix output  : ");
	handle_bool(output.matrix_output);
	(void) console_puts("\nMIDI output      : ");
	handle_bool(output.midi_output);
	if (output.midi_output) {
		printf(", baudrate : %d, interface : %s", (int) midi_get_baudrate(), midi_get_interface_description());
	}
	(void) console_puts("\nArtNet output    : ");
	handle_bool(output.artnet_output);

	for (;;) {
		switch (source) {
		case LTC_READER_SOURCE_LTC:
			ltc_reader();
			break;
		case LTC_READER_SOURCE_ARTNET:
			// Handles MIDI Quarter Frame output messages.
			reader.Run();
			break;
		case LTC_READER_SOURCE_MIDI:
			midi_reader();
			break;
		default:
			break;
		}

		if (output.artnet_output || (source == LTC_READER_SOURCE_ARTNET)) {
			// For all cases when ArtNet is enabled -> handles OpPoll / OpPollReply
			// When source == LTC_READER_SOURCE_ARTNET -> handle OpTimeCode
			(void) node.HandlePacket();
		}
	}
}

}
