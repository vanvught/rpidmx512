/**
 * @file wifi.c
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

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#include "esp8266.h"
#include "esp8266_cmd.h"
#include "wifi.h"

static char wifi_hostname[HOST_NAME_MAX + 1] __attribute__((aligned(4))) = { 'U' , 'n', 'k' , 'n' , 'o' , 'w', 'n' , '\0'};
static char sdk_version[SDK_VERSION_MAX + 1] __attribute__((aligned(4))) = { 'U' , 'n', 'k' , 'n' , 'o' , 'w', 'n' , '\0'};

/**
 *
 * @param mode
 * @return
 */
bool wifi_init(void) {
	esp8266_init();
	esp8266_write_4bits((uint8_t) CMD_NOP);
	return true;	// TODO
}

/**
 *
 * @return
 */
_wifi_mode wifi_get_opmode(void) {
	esp8266_write_4bits((uint8_t)CMD_WIFI_MODE);
	return esp8266_read_byte();
}

/**
 *
 * @param if_index
 * @param macaddr
 * @return
 */
bool wifi_get_macaddr(const uint8_t *macaddr) {
	assert(macaddr != NULL);

	if (macaddr == NULL) {
		return false;
	}

	esp8266_write_4bits((uint8_t)CMD_WIFI_MAC_ADDRESS);

	esp8266_read_bytes(macaddr, 6);	//TODO define

	return true;
}

/**
 *
 * @param if_index
 * @param info
 * @return
 */
bool wifi_get_ip_info(const struct ip_info *info) {
	assert(info != NULL);

	if (info == NULL) {
		return false;
	}

	esp8266_write_4bits((uint8_t)CMD_WIFI_IP_INFO);

	esp8266_read_bytes((const uint8_t *)info, sizeof(struct ip_info));

	return true;
}

/**
 *
 * @return
 */
const char *wifi_station_get_hostname(void) {
	unsigned i;
	uint8_t ch;

	esp8266_write_4bits((uint8_t) CMD_WIFI_HOST_NAME);

	i = 0;
	while ((ch = esp8266_read_byte()) != (uint8_t) 0) {
		if (i < HOST_NAME_MAX) {
			wifi_hostname[i] = (char) ch;
		}
		i++;
	}

	wifi_hostname[HOST_NAME_MAX] = '\0';

	return wifi_hostname;
}

/**
 *
 * @return
 */
const uint8_t system_get_cpu_freq(void) {
	esp8266_write_4bits((uint8_t)CMD_SYSTEM_CPU_FREQ);
	return esp8266_read_byte();
}

/**
 *
 * @return
 */
const char *system_get_sdk_version(void) {
	unsigned i;
	uint8_t ch;
	
	esp8266_write_4bits((uint8_t)CMD_SYSTEM_SDK_VERSION);
	
	i = 0;
	while((ch = esp8266_read_byte()) != (uint8_t) 0) {
		if (i < SDK_VERSION_MAX) {
			sdk_version[i] = (char) ch;
		}
		i++;
	}
	
	sdk_version[SDK_VERSION_MAX] = '\0';

	return sdk_version;
}

/**
 *
 * @param ssid
 * @param password
 */
void wifi_station(const char *ssid, const char *password) {
	assert(ssid != NULL);
	assert(password != NULL);

	esp8266_write_4bits((uint8_t)CMD_WIFI_MODE_STA);

	esp8266_write_str(ssid);
	esp8266_write_byte((uint8_t)(0));

	esp8266_write_str(password);
	esp8266_write_byte((uint8_t)(0));
}

/**
 *
 * @param ssid
 * @param ssid
 * @param
 */
void wifi_station_ip(const char *ssid, const char *password, const struct ip_info *info) {
	assert(ssid != NULL);
	assert(password != NULL);
	assert(password != NULL);

	esp8266_write_4bits((uint8_t)CMD_WIFI_MODE_STA_IP);

	esp8266_write_str(ssid);
	esp8266_write_byte((uint8_t)(0));

	esp8266_write_str(password);
	esp8266_write_byte((uint8_t)(0));

	esp8266_write_bytes((const uint8_t *)info, sizeof(struct ip_info));
}
