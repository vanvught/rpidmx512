/**
 * @file fota.cpp
 *
 */
/* Copyright (C) 2016-2021 by Arjan van Vught mailto:info@orangepi-dmx.nl
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

#include <cstdint>
#include <cassert>

#include "esp8266.h"
#include "esp8266_cmd.h"

#include "hardware.h"
#include "console.h"

static void esp8266_fota_start(const uint32_t server_ip_address) {
	esp8266_write_4bits(CMD_ESP_FOTA_START);
	esp8266_write_word(server_ip_address);
}

static void esp8266_fota_status(char *status, uint32_t *len) {
	assert(status != nullptr);
	assert(len != nullptr);

	esp8266_write_4bits(CMD_NOP);
	esp8266_read_str(status, len);
}

void fota(uint32_t server_ip_address) {
	char message[80];
	uint32_t nLength;
	char last_first_char = ' ';

	console_status(CONSOLE_YELLOW, "Starting FOTA ...");

	esp8266_fota_start(server_ip_address);

	do {
		nLength = sizeof(message) / sizeof(char);
		esp8266_fota_status(message, &nLength);
		if (nLength != 0) {
			console_puts(message);
			console_putc('\n');
			last_first_char = message[0];
		}
	} while (nLength != 0);

	if (last_first_char == 'S') {
		console_status(CONSOLE_GREEN, "FOTA Done!");
	} else {
		console_status(CONSOLE_RED, "FOTA Failed!");
	}

	for (;;)
		;
}
