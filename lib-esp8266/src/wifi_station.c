/**
 * @file wifi_station.c
 *
 */
/* Copyright (C) 2017 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
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

#include  "wifi.h"

#include "esp8266.h"
#include "esp8266_cmd.h"

static bool is_dhcp_used = false;

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
	esp8266_write_str(password);

	is_dhcp_used = true;
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
	assert(info != NULL);

	esp8266_write_4bits((uint8_t) CMD_WIFI_MODE_STA_IP);

	esp8266_write_str(ssid);
	esp8266_write_str(password);

	esp8266_write_bytes((const uint8_t *) info, sizeof(struct ip_info));

	is_dhcp_used = false;
}

/*
 *
 */
const _wifi_station_status wifi_station_get_connect_status(void) {
	esp8266_write_4bits((uint8_t) CMD_WIFI_MODE_STA_STATUS);
	return esp8266_read_byte();
}

/**
 *
 * @param status
 * @return
 */
const char *wifi_station_status(_wifi_station_status status) {
	switch (status) {
	case WIFI_STATION_IDLE:
		return "ESP8266 station idle";
		//break;
	case WIFI_STATION_CONNECTING:
		return "ESP8266 station is connecting to AP";
		//break;
	case WIFI_STATION_WRONG_PASSWORD:
		return "The password is wrong";
		//break;
	case WIFI_STATION_NO_AP_FOUND:
		return "ESP8266 station can not find the target AP";
		//break;
	case WIFI_STATION_CONNECT_FAIL:
		return "ESP8266 station fail to connect to AP";
		//break;
	case WIFI_STATION_GOT_IP:
		return "ESP8266 station got IP address from AP";
		//break;
	default:
		return "Unknown Status";
		//break;
	}

	//return "Unknown Status";
}

/**
 *
 * @return
 */
const bool wifi_station_is_dhcp_used(void) {
	return is_dhcp_used;
}

