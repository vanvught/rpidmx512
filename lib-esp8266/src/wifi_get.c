/**
 * @file wifi_get.c
 *
 */
/* Copyright (C) 2016-2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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
#include <stddef.h>
#include <stdbool.h>

#include "wifi.h"

#include "esp8266.h"
#include "esp8266_cmd.h"

static char firmware_version[FIRMWARE_VERSION_MAX + 1] __attribute__((aligned(4))) = { 'U' , 'n', 'k' , 'n' , 'o' , 'w', 'n' , '\0'};
static char wifi_hostname[HOST_NAME_MAX + 1] __attribute__((aligned(4))) = { 'U' , 'n', 'k' , 'n' , 'o' , 'w', 'n' , '\0'};

/**
 *
 * @return
 */
_wifi_mode wifi_get_opmode(void) {
	esp8266_write_4bits((uint8_t) CMD_WIFI_MODE);
	return esp8266_read_byte();
}

/**
 *
 * @param if_index
 * @param macaddr
 * @return
 */
const bool wifi_get_macaddr(const uint8_t *macaddr) {
	if (macaddr == NULL) {
		return false;
	}

	esp8266_write_4bits((uint8_t) CMD_WIFI_MAC_ADDRESS);
	esp8266_read_bytes(macaddr, 6);	//TODO define

	return true;
}

/**
 *
 * @param if_index
 * @param info
 * @return
 */
const bool wifi_get_ip_info(const struct ip_info *info) {
	if (info == NULL) {
		return false;
	}

	esp8266_write_4bits((uint8_t) CMD_WIFI_IP_INFO);
	esp8266_read_bytes((const uint8_t *) info, sizeof(struct ip_info));

	return true;
}

/**
 *
 * @return
 */
const char *wifi_get_hostname(void) {
	uint16_t len = HOST_NAME_MAX;

	esp8266_write_4bits((uint8_t) CMD_WIFI_HOST_NAME);
	esp8266_read_str(wifi_hostname, &len);

	return wifi_hostname;
}

/**
 *
 * @return
 */
const char *wifi_get_firmware_version(void) {
	uint16_t len = FIRMWARE_VERSION_MAX;

	esp8266_write_4bits((uint8_t)CMD_SYSTEM_FIRMWARE_VERSION);
	esp8266_read_str(firmware_version, &len);

	return firmware_version;
}
